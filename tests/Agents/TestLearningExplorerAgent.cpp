/**
 * @file TestLearningExplorerAgent.cpp
 * @brief Catch2 unit tests for LearningExplorerAgent.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/LearningExplorerAgent.hpp"
#include "../../source/Agents/EnemyAgent.hpp"
#include "../../source/Worlds/MazeWorld.hpp"

#include <cmath>
#include <set>

using namespace cse498;

// ============================================================
//  Initialization
// ============================================================

TEST_CASE("LearningExplorerAgent initializes with required actions",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  CHECK(agent.HasAction("up"));
  CHECK(agent.HasAction("down"));
  CHECK(agent.HasAction("left"));
  CHECK(agent.HasAction("right"));
}

TEST_CASE("LearningExplorerAgent has correct name and ID",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  CHECK(agent.GetName() == "Explorer");
  CHECK(agent.GetID() == 0);
}

// ============================================================
//  SelectAction basics
// ============================================================

TEST_CASE("LearningExplorerAgent SelectAction returns non-zero action",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  size_t action = agent.SelectAction(world.GetGrid());
  CHECK(action != 0);
}

TEST_CASE("LearningExplorerAgent GetLastAction tracks selected action",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  CHECK(agent.GetLastAction() == 0);

  size_t action = agent.SelectAction(world.GetGrid());
  CHECK(agent.GetLastAction() == action);
}

// ============================================================
//  Visit count tracking
// ============================================================

TEST_CASE("LearningExplorerAgent visit count starts at zero",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  CHECK(agent.GetVisitedCellCount(world.GetGrid(), WorldPosition{6, 1}) == 0);
  CHECK(agent.GetVisitedCellCount(world.GetGrid(), WorldPosition{7, 1}) == 0);
}

TEST_CASE("LearningExplorerAgent increments visit count on SelectAction",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  (void)agent.SelectAction(world.GetGrid());
  CHECK(agent.GetVisitedCellCount(world.GetGrid(), WorldPosition{6, 1}) == 1);

  // Move and come back: visit count should increase
  size_t action = agent.GetLastAction();
  world.DoAction(agent, action);
  agent.SetActionResult(1);
  (void)agent.SelectAction(world.GetGrid());

  // The new cell was visited once
  WorldPosition new_pos = agent.GetLocation().AsWorldPosition();
  CHECK(agent.GetVisitedCellCount(world.GetGrid(), new_pos) >= 1);
}

// ============================================================
//  Success / failure stubs
// ============================================================

TEST_CASE("LearningExplorerAgent success and failure counts return zero",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  CHECK(agent.GetActionSuccessCount(1) == 0);
  CHECK(agent.GetActionSuccessCount(2) == 0);
  CHECK(agent.GetActionFailureCount(1) == 0);
  CHECK(agent.GetActionFailureCount(2) == 0);
}

// ============================================================
//  Movement and exploration
// ============================================================

TEST_CASE("LearningExplorerAgent moves to a valid floor tile",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  size_t action = agent.SelectAction(world.GetGrid());
  int result = world.DoAction(agent, action);
  agent.SetActionResult(result);

  WorldPosition pos = agent.GetLocation().AsWorldPosition();
  CHECK(world.GetGrid().IsValid(pos));
  size_t wall_id = world.GetGrid().GetCellTypeID("wall");
  CHECK(world.GetGrid()[pos] != wall_id);
}

TEST_CASE("LearningExplorerAgent explores multiple cells over time",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  std::set<std::pair<size_t, size_t>> visited;

  for (int step = 0; step < 50; ++step) {
    size_t action = agent.SelectAction(world.GetGrid());
    int result = world.DoAction(agent, action);
    agent.SetActionResult(result);

    WorldPosition pos = agent.GetLocation().AsWorldPosition();
    visited.insert({pos.CellX(), pos.CellY()});
  }

  CHECK(visited.size() > 10);
}

TEST_CASE("LearningExplorerAgent never walks into a wall",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  const size_t wall_id = world.GetGrid().GetCellTypeID("wall");

  for (int step = 0; step < 100; ++step) {
    size_t action = agent.SelectAction(world.GetGrid());
    int result = world.DoAction(agent, action);
    agent.SetActionResult(result);

    WorldPosition pos = agent.GetLocation().AsWorldPosition();
    REQUIRE(world.GetGrid().IsValid(pos));
    REQUIRE(world.GetGrid()[pos] != wall_id);
  }
}

// ============================================================
//  Agent avoidance
// ============================================================

TEST_CASE("LearningExplorerAgent avoids cell occupied by another agent",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &explorer = world.AddAgent<LearningExplorerAgent>("Explorer");
  explorer.SetLocation(WorldPosition{10, 7});

  auto &blocker = world.AddAgent<LearningExplorerAgent>("Blocker");
  blocker.SetLocation(WorldPosition{11, 7});

  size_t action = explorer.SelectAction(world.GetGrid());
  int result = world.DoAction(explorer, action);
  explorer.SetActionResult(result);

  WorldPosition new_pos = explorer.GetLocation().AsWorldPosition();
  WorldPosition blocker_pos = blocker.GetLocation().AsWorldPosition();
  CHECK_FALSE(new_pos == blocker_pos);
}

TEST_CASE("LearningExplorerAgent avoids proximity to enemy agent",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &explorer = world.AddAgent<LearningExplorerAgent>("Explorer");
  explorer.SetLocation(WorldPosition{10, 7});

  auto &enemy = world.AddAgent<EnemyAgent>("Enemy");
  enemy.SetLocation(WorldPosition{12, 7});

  size_t action = explorer.SelectAction(world.GetGrid());
  int result = world.DoAction(explorer, action);
  explorer.SetActionResult(result);

  WorldPosition new_pos = explorer.GetLocation().AsWorldPosition();
  WorldPosition enemy_pos = enemy.GetLocation().AsWorldPosition();

  int dx = static_cast<int>(new_pos.CellX()) -
           static_cast<int>(enemy_pos.CellX());
  int dy = static_cast<int>(new_pos.CellY()) -
           static_cast<int>(enemy_pos.CellY());
  int dist = std::abs(dx) + std::abs(dy);

  CHECK(dist >= 2);
}
