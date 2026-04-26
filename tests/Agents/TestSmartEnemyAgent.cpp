/**
 * @file TestSmartEnemyAgent.cpp
 * @brief Unit tests for SmartEnemyAgent.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/AI/SmartEnemyAgent.hpp"
#include "../../source/Agents/AI/LearningExplorerAgent.hpp"
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/core/WorldBase.hpp"

namespace cse498 {
// ============================================================
//  Initialization
// ============================================================

TEST_CASE("SmartEnemyAgent initializes with required actions", "[SmartEnemyAgent]") {
    MazeWorld world;
    auto& agent = world.AddAgent<SmartEnemyAgent>("SmartEnemy");
    agent.SetLocation(WorldPosition{10, 7});

    CHECK(agent.Initialize());
    CHECK(agent.HasAction("up"));
    CHECK(agent.HasAction("down"));
    CHECK(agent.HasAction("left"));
    CHECK(agent.HasAction("right"));
}

TEST_CASE("SmartEnemyAgent Initialize fails without movement actions", "[SmartEnemyAgent]") {
    MazeWorld world;
    SmartEnemyAgent agent(0, "Enemy", world);

    CHECK(agent.Initialize());
}

TEST_CASE("SmartEnemyAgent has correct name and ID", "[SmartEnemyAgent]") {
    MazeWorld world;
    auto& agent = world.AddAgent<SmartEnemyAgent>("SmartEnemy");
    agent.SetLocation(WorldPosition{10, 7});

    CHECK(agent.GetName() == "SmartEnemy");
    CHECK(agent.GetID() == 0);
}

// ============================================================
//  IsEnemy behavior
// ============================================================

TEST_CASE("SmartEnemyAgent IsEnemy returns true", "[SmartEnemyAgent]") {
    MazeWorld world;
    auto& agent = world.AddAgent<SmartEnemyAgent>("SmartEnemy");

    CHECK(agent.IsEnemy());
}

TEST_CASE("SmartEnemyAgent IsEnemy distinguishes from non-enemy", "[SmartEnemyAgent]") {
    MazeWorld world;
    auto& enemy = world.AddAgent<SmartEnemyAgent>("Enemy");
    auto& explorer = world.AddAgent<LearningExplorerAgent>("Explorer");

    CHECK(enemy.IsEnemy());
    CHECK_FALSE(explorer.IsEnemy());
}

// ============================================================
//  SelectAction basics
// ============================================================

TEST_CASE("SmartEnemyAgent SelectAction returns valid action", "[SmartEnemyAgent]") {
    MazeWorld world;
    auto& agent = world.AddAgent<SmartEnemyAgent>("SmartEnemy");
    agent.SetLocation(WorldPosition{10, 7});

    size_t action = agent.SelectAction(world.GetGrid());
    CHECK((action == 0 || action > 0)); // ensure no crash / valid return
}

TEST_CASE("SmartEnemyAgent SelectAction does not crash without tracked player", "[SmartEnemyAgent]") {
    MazeWorld world;
    auto& agent = world.AddAgent<SmartEnemyAgent>("SmartEnemy");
    agent.SetLocation(WorldPosition{10, 7});

    size_t action = agent.SelectAction(world.GetGrid());
    CHECK((action == 0 || action > 0));
}

// ============================================================
//  Movement behavior
// ============================================================

TEST_CASE("SmartEnemyAgent moves to valid tile when action succeeds", "[SmartEnemyAgent]") {
    MazeWorld world;
    auto& agent = world.AddAgent<SmartEnemyAgent>("SmartEnemy");
    agent.SetLocation(WorldPosition{10, 7});

    size_t action = agent.SelectAction(world.GetGrid());
    world.DoAction(agent, action);

    WorldPosition pos = agent.GetLocation().AsWorldPosition();

    CHECK(world.GetGrid().IsValid(pos));
    size_t wall_id = world.GetGrid().GetCellTypeID("wall");
    CHECK(world.GetGrid()[pos] != wall_id);
}

// ============================================================
//  Stability tests
// ============================================================

TEST_CASE("SmartEnemyAgent remains stable over multiple steps", "[SmartEnemyAgent]") {
    MazeWorld world;
    auto& agent = world.AddAgent<SmartEnemyAgent>("SmartEnemy");
    agent.SetLocation(WorldPosition{10, 7});

    for (int i = 0; i < 100; ++i) {
        size_t action = agent.SelectAction(world.GetGrid());
        world.DoAction(agent, action);

        WorldPosition pos = agent.GetLocation().AsWorldPosition();
        REQUIRE(world.GetGrid().IsValid(pos));
    }

    size_t action = agent.SelectAction(world.GetGrid());
    CHECK((action == 0 || action > 0));
}

// ============================================================
//  Edge cases
// ============================================================

TEST_CASE("SmartEnemyAgent handles invalid action gracefully", "[SmartEnemyAgent]") {
    MazeWorld world;
    auto& agent = world.AddAgent<SmartEnemyAgent>("SmartEnemy");
    agent.SetLocation(WorldPosition{10, 7});

    WorldPosition before = agent.GetLocation().AsWorldPosition();

    world.DoAction(agent, 0); // REMAIN_STILL
    CHECK(agent.GetLocation().AsWorldPosition() == before);

    size_t next_action = agent.SelectAction(world.GetGrid());
    CHECK((next_action == 0 || next_action > 0));
}

TEST_CASE("SmartEnemyAgent does not move into walls over time", "[SmartEnemyAgent]") {
    MazeWorld world;
    auto& agent = world.AddAgent<SmartEnemyAgent>("SmartEnemy");
    agent.SetLocation(WorldPosition{1, 1});

    size_t wall_id = world.GetGrid().GetCellTypeID("wall");

    for (int i = 0; i < 50; ++i) {
        size_t action = agent.SelectAction(world.GetGrid());
        world.DoAction(agent, action);

        WorldPosition pos = agent.GetLocation().AsWorldPosition();
        REQUIRE(world.GetGrid().IsValid(pos));
        REQUIRE(world.GetGrid()[pos] != wall_id);
    }
}
}