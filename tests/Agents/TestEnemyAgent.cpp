/**
 * @file TestEnemyAgent.cpp
 * @brief Catch2 unit tests for EnemyAgent.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/AI/EnemyAgent.hpp"
#include "../../source/Agents/AI/LearningExplorerAgent.hpp"
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/core/WorldBase.hpp"

#include <cmath>
#include <set>

using namespace cse498;

namespace {
/// World that never registers movement actions (for Initialize() failure tests).
struct NoMovementActionWorld : WorldBase {
    void ConfigAgent(AgentBase&) override {}
    int DoAction(AgentBase&, size_t) override { return 0; }
};
} // namespace

// ============================================================
//  Initialization
// ============================================================

TEST_CASE("EnemyAgent initializes with required actions", "[EnemyAgent]") {
    MazeWorld world;
    auto& agent = world.AddAgent<EnemyAgent>("Enemy");
    agent.SetLocation(WorldPosition{10, 7});

    CHECK(agent.Initialize());
    CHECK(agent.HasAction("up"));
    CHECK(agent.HasAction("down"));
    CHECK(agent.HasAction("left"));
    CHECK(agent.HasAction("right"));
}

TEST_CASE("EnemyAgent initializes as an enemy", "[EnemyAgent]") {
    MazeWorld world;
    auto& agent = world.AddAgent<EnemyAgent>("Enemy");
    agent.SetLocation(WorldPosition{10, 7});

    CHECK(agent.Initialize());
    CHECK(agent.IsEnemy());
}

TEST_CASE("EnemyAgent Initialize fails without movement actions", "[EnemyAgent]") {
    NoMovementActionWorld world;
    EnemyAgent agent(0, "E", world);
    CHECK_FALSE(agent.Initialize());
}

TEST_CASE("EnemyAgent has correct name and ID", "[EnemyAgent]") {
    MazeWorld world;
    world.AddAgent<LearningExplorerAgent>("Explorer").SetLocation(WorldPosition{1, 7});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{10, 7});

    CHECK(enemy.GetName() == "Enemy");
    CHECK(enemy.GetID() == 1);
}

// ============================================================
//  SelectAction basics
// ============================================================

TEST_CASE("EnemyAgent SelectAction returns non-zero action", "[EnemyAgent]") {
    MazeWorld world;
    world.AddAgent<LearningExplorerAgent>("Target").SetLocation(WorldPosition{5, 7});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{10, 7});

    size_t action = enemy.SelectAction(world.GetGrid());
    CHECK(action != 0);
}

// ============================================================
//  Chasing behaviour
// ============================================================

TEST_CASE("EnemyAgent moves closer to target on one step", "[EnemyAgent]") {
    MazeWorld world;
    auto& target = world.AddAgent<LearningExplorerAgent>("Target");
    target.SetLocation(WorldPosition{1, 7});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{10, 7});

    WorldPosition start = enemy.GetLocation().AsWorldPosition();
    WorldPosition target_pos = target.GetLocation().AsWorldPosition();

    size_t action = enemy.SelectAction(world.GetGrid());
    world.DoAction(enemy, action);

    WorldPosition after = enemy.GetLocation().AsWorldPosition();

    int old_dist = std::abs(static_cast<int>(start.CellX()) - static_cast<int>(target_pos.CellX())) +
                   std::abs(static_cast<int>(start.CellY()) - static_cast<int>(target_pos.CellY()));

    int new_dist = std::abs(static_cast<int>(after.CellX()) - static_cast<int>(target_pos.CellX())) +
                   std::abs(static_cast<int>(after.CellY()) - static_cast<int>(target_pos.CellY()));

    CHECK(new_dist < old_dist);
}

TEST_CASE("EnemyAgent pursues over multiple steps", "[EnemyAgent]") {
    MazeWorld world;
    auto& target = world.AddAgent<LearningExplorerAgent>("Target");
    target.SetLocation(WorldPosition{1, 7});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{15, 7});

    WorldPosition initial = enemy.GetLocation().AsWorldPosition();
    WorldPosition target_pos = target.GetLocation().AsWorldPosition();

    for (int step = 0; step < 10; ++step) {
        size_t action = enemy.SelectAction(world.GetGrid());
        int result = world.DoAction(enemy, action);
        enemy.SetActionResult(result);
    }

    WorldPosition final_pos = enemy.GetLocation().AsWorldPosition();

    int initial_dist = std::abs(static_cast<int>(initial.CellX()) - static_cast<int>(target_pos.CellX())) +
                       std::abs(static_cast<int>(initial.CellY()) - static_cast<int>(target_pos.CellY()));

    int final_dist = std::abs(static_cast<int>(final_pos.CellX()) - static_cast<int>(target_pos.CellX())) +
                     std::abs(static_cast<int>(final_pos.CellY()) - static_cast<int>(target_pos.CellY()));

    CHECK(final_dist < initial_dist);
}

// ============================================================
//  Wall avoidance
// ============================================================

TEST_CASE("EnemyAgent never occupies a wall cell", "[EnemyAgent]") {
    MazeWorld world;
    auto& target = world.AddAgent<LearningExplorerAgent>("Target");
    target.SetLocation(WorldPosition{1, 1});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{1, 7});

    const size_t wall_id = world.GetGrid().GetCellTypeID("wall");

    for (int step = 0; step < 30; ++step) {
        size_t action = enemy.SelectAction(world.GetGrid());
        int result = world.DoAction(enemy, action);
        enemy.SetActionResult(result);

        WorldPosition pos = enemy.GetLocation().AsWorldPosition();
        REQUIRE(world.GetGrid().IsValid(pos));
        REQUIRE(world.GetGrid()[pos] != wall_id);
    }
}

// ============================================================
//  Movement validity
// ============================================================

TEST_CASE("EnemyAgent always stays on a valid grid position", "[EnemyAgent]") {
    MazeWorld world;
    auto& target = world.AddAgent<LearningExplorerAgent>("Target");
    target.SetLocation(WorldPosition{15, 7});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{5, 7});

    for (int step = 0; step < 40; ++step) {
        size_t action = enemy.SelectAction(world.GetGrid());
        int result = world.DoAction(enemy, action);
        enemy.SetActionResult(result);

        WorldPosition pos = enemy.GetLocation().AsWorldPosition();
        CHECK(world.GetGrid().IsValid(pos));
    }
}

TEST_CASE("EnemyAgent chases horizontally in open corridor", "[EnemyAgent]") {
    MazeWorld world;
    auto& target = world.AddAgent<LearningExplorerAgent>("Target");
    target.SetLocation(WorldPosition{1, 8});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{10, 8});

    size_t action = enemy.SelectAction(world.GetGrid());
    world.DoAction(enemy, action);

    WorldPosition after = enemy.GetLocation().AsWorldPosition();

    // In a straight horizontal corridor, enemy should move left toward target.
    CHECK(after.CellX() == 9);
    CHECK(after.CellY() == 8);
}

TEST_CASE("EnemyAgent chases vertically in open corridor", "[EnemyAgent]") {
    MazeWorld world;
    world.AddAgent<LearningExplorerAgent>("Target").SetLocation(WorldPosition{10, 2});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{10, 8});

    size_t action = enemy.SelectAction(world.GetGrid());
    world.DoAction(enemy, action);

    WorldPosition after = enemy.GetLocation().AsWorldPosition();
    CHECK(after.CellX() == 10);
    CHECK(after.CellY() == 7);
}

TEST_CASE("EnemyAgent with no other agent still selects valid move", "[EnemyAgent]") {
    MazeWorld world;
    auto& enemy = world.AddAgent<EnemyAgent>("Solo");
    enemy.SetLocation(WorldPosition{10, 7});

    size_t action = enemy.SelectAction(world.GetGrid());
    CHECK(action != 0);
    int result = world.DoAction(enemy, action);
    enemy.SetActionResult(result);
    WorldPosition pos = enemy.GetLocation().AsWorldPosition();
    CHECK(world.GetGrid().IsValid(pos));
    CHECK(world.GetGrid()[pos] != world.GetGrid().GetCellTypeID("wall"));
}

// ============================================================
//  Initialize edge cases
// ============================================================

TEST_CASE("EnemyAgent Initialize returns true only when all four actions exist", "[EnemyAgent]") {
    MazeWorld world;
    auto& agent = world.AddAgent<EnemyAgent>("Enemy");
    agent.SetLocation(WorldPosition{10, 7});

    // MazeWorld adds all four actions, so Initialize should pass
    CHECK(agent.Initialize());

    // Verify each action individually
    CHECK(agent.HasAction("up"));
    CHECK(agent.HasAction("down"));
    CHECK(agent.HasAction("left"));
    CHECK(agent.HasAction("right"));

    // Non-existent actions should return false
    CHECK_FALSE(agent.HasAction("jump"));
    CHECK_FALSE(agent.HasAction(""));
    CHECK_FALSE(agent.HasAction("fly"));
}

TEST_CASE("EnemyAgent Initialize can be called multiple times safely", "[EnemyAgent]") {
    MazeWorld world;
    auto& agent = world.AddAgent<EnemyAgent>("Enemy");
    agent.SetLocation(WorldPosition{10, 7});

    CHECK(agent.Initialize());
    CHECK(agent.Initialize());
    CHECK(agent.Initialize());
}

// ============================================================
//  SetActionResult edge cases
// ============================================================

TEST_CASE("EnemyAgent SetActionResult with failure does not corrupt state", "[EnemyAgent]") {
    MazeWorld world;
    auto& target = world.AddAgent<LearningExplorerAgent>("Target");
    target.SetLocation(WorldPosition{1, 7});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{10, 7});

    WorldPosition before = enemy.GetLocation().AsWorldPosition();

    // Force a move into a wall (MOVE_UP from row 7 toward walls)
    int result = world.DoAction(enemy, 1); // MOVE_UP
    enemy.SetActionResult(result);

    // If move failed, position should be unchanged
    if (result == 0) {
        CHECK(enemy.GetLocation().AsWorldPosition() == before);
    }

    // Agent should still function normally
    size_t action = enemy.SelectAction(world.GetGrid());
    CHECK(action != 0);
}

TEST_CASE("EnemyAgent SetActionResult with success updates position", "[EnemyAgent]") {
    MazeWorld world;
    auto& target = world.AddAgent<LearningExplorerAgent>("Target");
    target.SetLocation(WorldPosition{1, 7});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{10, 7});

    WorldPosition before = enemy.GetLocation().AsWorldPosition();

    size_t action = enemy.SelectAction(world.GetGrid());
    int result = world.DoAction(enemy, action);
    enemy.SetActionResult(result);

    CHECK(result == 1);
    // Position should have changed after a successful move
    CHECK_FALSE(enemy.GetLocation().AsWorldPosition() == before);
}

TEST_CASE("EnemyAgent handles repeated failed actions without crashing", "[EnemyAgent]") {
    MazeWorld world;
    auto& target = world.AddAgent<LearningExplorerAgent>("Target");
    target.SetLocation(WorldPosition{1, 7});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{1, 1});

    WorldPosition start = enemy.GetLocation().AsWorldPosition();

    // Repeatedly try to move up into the wall
    for (int i = 0; i < 10; ++i) {
        int result = world.DoAction(enemy, 1); // MOVE_UP into wall
        enemy.SetActionResult(result);
        CHECK(result == 0);
    }

    // Agent should be at the same position
    CHECK(enemy.GetLocation().AsWorldPosition() == start);

    // Agent should still select valid actions
    size_t action = enemy.SelectAction(world.GetGrid());
    CHECK(action != 0);
}

// ============================================================
//  Invalid action IDs and edge cases
// ============================================================

TEST_CASE("EnemyAgent action ID 0 (REMAIN_STILL) keeps position unchanged", "[EnemyAgent]") {
    MazeWorld world;
    auto& target = world.AddAgent<LearningExplorerAgent>("Target");
    target.SetLocation(WorldPosition{1, 7});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{10, 7});

    WorldPosition before = enemy.GetLocation().AsWorldPosition();

    int result = world.DoAction(enemy, 0); // REMAIN_STILL
    enemy.SetActionResult(result);

    CHECK(enemy.GetLocation().AsWorldPosition() == before);
}

// ============================================================
//  Getter coverage
// ============================================================

TEST_CASE("EnemyAgent GetName returns correct name", "[EnemyAgent]") {
    MazeWorld world;
    auto& enemy = world.AddAgent<EnemyAgent>("TestEnemy");
    enemy.SetLocation(WorldPosition{10, 7});
    CHECK(enemy.GetName() == "TestEnemy");
}

TEST_CASE("EnemyAgent GetID returns sequential IDs", "[EnemyAgent]") {
    MazeWorld world;
    auto& first = world.AddAgent<EnemyAgent>("First");
    first.SetLocation(WorldPosition{10, 7});

    auto& second = world.AddAgent<EnemyAgent>("Second");
    second.SetLocation(WorldPosition{5, 7});

    CHECK(first.GetID() == 0);
    CHECK(second.GetID() == 1);
}

TEST_CASE("EnemyAgent GetLocation returns set position", "[EnemyAgent]") {
    MazeWorld world;
    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{10, 7});

    WorldPosition pos = enemy.GetLocation().AsWorldPosition();
    CHECK(pos.CellX() == 10);
    CHECK(pos.CellY() == 7);
}

TEST_CASE("EnemyAgent SetSymbol and GetSymbol", "[EnemyAgent]") {
    MazeWorld world;
    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{10, 7});

    enemy.SetSymbol('X');
    CHECK(enemy.GetSymbol() == 'X');

    enemy.SetSymbol('E');
    CHECK(enemy.GetSymbol() == 'E');
}

// ============================================================
//  Cornered / boundary cases
// ============================================================

TEST_CASE("EnemyAgent cornered near walls still selects valid action", "[EnemyAgent]") {
    MazeWorld world;
    auto& target = world.AddAgent<LearningExplorerAgent>("Target");
    target.SetLocation(WorldPosition{15, 7});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    // Top-left corner area — walls above and to the left
    enemy.SetLocation(WorldPosition{1, 1});

    size_t action = enemy.SelectAction(world.GetGrid());
    CHECK(action != 0);

    int result = world.DoAction(enemy, action);
    enemy.SetActionResult(result);

    WorldPosition pos = enemy.GetLocation().AsWorldPosition();
    CHECK(world.GetGrid().IsValid(pos));
    CHECK(world.GetGrid()[pos] != world.GetGrid().GetCellTypeID("wall"));
}

TEST_CASE("EnemyAgent does not walk into walls after many steps at boundary", "[EnemyAgent]") {
    MazeWorld world;
    auto& target = world.AddAgent<LearningExplorerAgent>("Target");
    target.SetLocation(WorldPosition{20, 5});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{1, 1});

    const size_t wall_id = world.GetGrid().GetCellTypeID("wall");

    for (int step = 0; step < 50; ++step) {
        size_t action = enemy.SelectAction(world.GetGrid());
        int result = world.DoAction(enemy, action);
        enemy.SetActionResult(result);

        WorldPosition pos = enemy.GetLocation().AsWorldPosition();
        REQUIRE(world.GetGrid().IsValid(pos));
        REQUIRE(world.GetGrid()[pos] != wall_id);
    }
}

TEST_CASE("EnemyAgent remains stable after long chase run", "[EnemyAgent]") {
    MazeWorld world;
    auto& target = world.AddAgent<LearningExplorerAgent>("Target");
    target.SetLocation(WorldPosition{1, 7});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{15, 7});

    for (int step = 0; step < 200; ++step) {
        size_t action = enemy.SelectAction(world.GetGrid());
        int result = world.DoAction(enemy, action);
        enemy.SetActionResult(result);
    }

    // Agent should still produce valid actions
    size_t action = enemy.SelectAction(world.GetGrid());
    CHECK(action != 0);
}
