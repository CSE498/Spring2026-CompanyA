/**
 * @file TestTrailblazerAgent.cpp
 * @brief Catch2 unit tests for TrailblazerAgent (AIWorld integration).
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/TrailblazerAgent.hpp"
#include "../../source/Worlds/AIWorld.hpp"
#include "../../source/Worlds/MazeWorld.hpp"

using namespace cse498;

namespace {
struct AIWorldStateHarness : AIWorld {
public:
  using AIWorld::mAgentState;
  using AIWorld::mEnemies;
};
} // namespace

TEST_CASE("TrailblazerAgent Initialize sets symbol and succeeds",
          "[TrailblazerAgent]") {
  AIWorld world;
  auto &agent = world.AddAgent<TrailblazerAgent>("Hero");
  CHECK(agent.Initialize());
  CHECK(agent.GetSymbol() == 'T');
}

TEST_CASE("TrailblazerAgent SelectAction returns zero outside AIWorld",
          "[TrailblazerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<TrailblazerAgent>("Hero");
  agent.SetLocation(WorldPosition{10, 7});
  CHECK(agent.SelectAction(world.GetGrid()) == 0);
}

TEST_CASE("TrailblazerAgent attacks adjacent enemy to the east", "[TrailblazerAgent]") {
  AIWorld world;
  world.SetVerboseMode(false);
  world.SetStepMode(false);

  auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
  hero.SetLocation(WorldPosition{4, 5});

  size_t action = hero.SelectAction(world.GetGrid());
  CHECK(action == hero.GetActionID("attack_right"));
}

TEST_CASE("TrailblazerAgent attacks adjacent enemy in all cardinal directions",
          "[TrailblazerAgent]") {
  AIWorld world;
  world.SetVerboseMode(false);
  world.SetStepMode(false);

  struct Case {
    WorldPosition agent;
    const char *expected_attack;
  };
  // Default enemy #0 sits at (5, 5).
  const Case cases[] = {
      {WorldPosition{5, 4}, "attack_down"},
      {WorldPosition{5, 6}, "attack_up"},
      {WorldPosition{4, 5}, "attack_right"},
      {WorldPosition{6, 5}, "attack_left"},
  };

  for (const auto &c : cases) {
    AIWorld w;
    w.SetVerboseMode(false);
    w.SetStepMode(false);
    auto &a = w.AddAgent<TrailblazerAgent>("Hero");
    a.SetLocation(c.agent);
    size_t act = a.SelectAction(w.GetGrid());
    CHECK(act == a.GetActionID(c.expected_attack));
  }
}

TEST_CASE("TrailblazerAgent picks up loot when standing on item tile",
          "[TrailblazerAgent]") {
  AIWorld world;
  world.SetVerboseMode(false);
  world.SetStepMode(false);

  auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
  hero.SetLocation(WorldPosition{3, 5});

  size_t action = hero.SelectAction(world.GetGrid());
  CHECK(action == hero.GetActionID("pickup"));
}

TEST_CASE("TrailblazerAgent plans use_heal when hurt and holding charges",
          "[TrailblazerAgent]") {
  AIWorldStateHarness world;
  world.SetVerboseMode(false);
  world.SetStepMode(false);

  auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
  hero.SetLocation(WorldPosition{10, 7});

  auto &st = world.mAgentState.at(hero.GetID());
  st.mHP = 3;
  st.mMaxHP = 12;
  st.mHealCharges = 4;

  size_t action = hero.SelectAction(world.GetGrid());
  CHECK(action == hero.GetActionID("use_heal"));
}

TEST_CASE("TrailblazerAgent explores when all enemies are defeated",
          "[TrailblazerAgent]") {
  AIWorldStateHarness world;
  world.SetVerboseMode(false);
  world.SetStepMode(false);

  for (auto &enemy : world.mEnemies) {
    enemy.mAlive = false;
  }

  auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
  hero.SetLocation(WorldPosition{10, 7});

  size_t action = hero.SelectAction(world.GetGrid());
  CHECK(action != 0);
  const bool is_move =
      (action == hero.GetActionID("up")) || (action == hero.GetActionID("down")) ||
      (action == hero.GetActionID("left")) || (action == hero.GetActionID("right"));
  CHECK(is_move);
}
