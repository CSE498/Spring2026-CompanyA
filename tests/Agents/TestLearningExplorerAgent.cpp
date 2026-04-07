/**
 * @file TestLearningExplorerAgent.cpp
 * @brief Catch2 unit tests for LearningExplorerAgent.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/LearningExplorerAgent.hpp"
#include "../../source/Agents/EnemyAgent.hpp"
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/core/WorldBase.hpp"

#include <cmath>
#include <set>

using namespace cse498;

namespace {
struct NoMovementActionWorld : WorldBase {
  void ConfigAgent(AgentBase &) override {}
  int DoAction(AgentBase &, size_t) override { return 0; }
};
} // namespace

// ============================================================
//  Initialization
// ============================================================

TEST_CASE("LearningExplorerAgent initializes with required actions",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  CHECK(agent.Initialize());
  CHECK(agent.HasAction("up"));
  CHECK(agent.HasAction("down"));
  CHECK(agent.HasAction("left"));
  CHECK(agent.HasAction("right"));
}

TEST_CASE("LearningExplorerAgent Initialize fails without movement actions",
          "[LearningExplorerAgent]") {
  NoMovementActionWorld world;
  LearningExplorerAgent agent(0, "E", world);
  CHECK_FALSE(agent.Initialize());
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

TEST_CASE("LearningExplorerAgent visit count increases when SelectAction called "
          "without moving",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  (void)agent.SelectAction(world.GetGrid());
  (void)agent.SelectAction(world.GetGrid());
  CHECK(agent.GetVisitedCellCount(world.GetGrid(), WorldPosition{6, 1}) >= 2);
}

TEST_CASE("LearningExplorerAgent remains stable after long exploration run",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  for (int step = 0; step < 400; ++step) {
    size_t action = agent.SelectAction(world.GetGrid());
    int result = world.DoAction(agent, action);
    agent.SetActionResult(result);
  }

  size_t action = agent.SelectAction(world.GetGrid());
  CHECK(action != 0);
}

// ============================================================
//  Invalid states and error conditions
// ============================================================

TEST_CASE("LearningExplorerAgent recovers from failed move into wall",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  // Position adjacent to top wall row
  agent.SetLocation(WorldPosition{1, 1});

  WorldPosition start_pos = agent.GetLocation().AsWorldPosition();

  // Try to force a move up into the wall (row 0 is all walls)
  int result = world.DoAction(agent, 1); // MOVE_UP
  agent.SetActionResult(result);

  // Move should fail and agent should stay in place
  CHECK(result == 0);
  CHECK(agent.GetLocation().AsWorldPosition() == start_pos);

  // Agent should still be able to select a valid action afterward
  size_t next_action = agent.SelectAction(world.GetGrid());
  CHECK(next_action != 0);
}

TEST_CASE("LearningExplorerAgent handles repeated failed actions gracefully",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{1, 1});

  WorldPosition start_pos = agent.GetLocation().AsWorldPosition();

  // Repeatedly try to move up into a wall
  for (int i = 0; i < 10; ++i) {
    int result = world.DoAction(agent, 1); // MOVE_UP
    agent.SetActionResult(result);
    CHECK(result == 0);
  }

  // Agent should still be at original position
  CHECK(agent.GetLocation().AsWorldPosition() == start_pos);

  // Agent should still produce valid actions
  size_t action = agent.SelectAction(world.GetGrid());
  CHECK(action != 0);
}

TEST_CASE("LearningExplorerAgent cornered by walls still selects valid action",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  // Top-left corner: walls above and to the left, wall below (row 0,1 col 0 are walls)
  // Place at (1,1) which is a floor cell cornered by walls on two sides
  agent.SetLocation(WorldPosition{1, 1});

  // The agent should still find a valid action (down or right)
  size_t action = agent.SelectAction(world.GetGrid());
  CHECK(action != 0);

  int result = world.DoAction(agent, action);
  agent.SetActionResult(result);

  // The agent should successfully move to a valid cell
  WorldPosition new_pos = agent.GetLocation().AsWorldPosition();
  CHECK(world.GetGrid().IsValid(new_pos));
  CHECK(world.GetGrid()[new_pos] != world.GetGrid().GetCellTypeID("wall"));
}

TEST_CASE("LearningExplorerAgent surrounded by agents still selects action",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &explorer = world.AddAgent<LearningExplorerAgent>("Explorer");
  explorer.SetLocation(WorldPosition{10, 7});

  // Surround the explorer with other agents on all four sides
  auto &up = world.AddAgent<LearningExplorerAgent>("Up");
  up.SetLocation(WorldPosition{10, 6});
  auto &down = world.AddAgent<LearningExplorerAgent>("Down");
  down.SetLocation(WorldPosition{10, 8});
  auto &left = world.AddAgent<LearningExplorerAgent>("Left");
  left.SetLocation(WorldPosition{9, 7});
  auto &right = world.AddAgent<LearningExplorerAgent>("Right");
  right.SetLocation(WorldPosition{11, 7});

  // Agent should still return a non-zero action (even if the move will fail)
  size_t action = explorer.SelectAction(world.GetGrid());
  CHECK(action != 0);
}

TEST_CASE("LearningExplorerAgent action result of failure does not corrupt state",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  // Do a successful action
  size_t action1 = agent.SelectAction(world.GetGrid());
  int result1 = world.DoAction(agent, action1);
  agent.SetActionResult(result1);
  CHECK(result1 == 1);

  // Now force a failure by trying to move into a wall
  int fail_result = world.DoAction(agent, 1); // MOVE_UP toward potential wall
  agent.SetActionResult(fail_result);

  // Whether it succeeded or failed, position should be valid
  WorldPosition after_attempt = agent.GetLocation().AsWorldPosition();
  CHECK(world.GetGrid().IsValid(after_attempt));

  // Agent should still select valid actions
  size_t action2 = agent.SelectAction(world.GetGrid());
  CHECK(action2 != 0);
}

TEST_CASE("LearningExplorerAgent handles invalid action ID from DoAction",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  WorldPosition start_pos = agent.GetLocation().AsWorldPosition();

  // Action ID 0 is REMAIN_STILL — agent should not move
  int result = world.DoAction(agent, 0);
  agent.SetActionResult(result);

  CHECK(agent.GetLocation().AsWorldPosition() == start_pos);

  // Agent should still function normally after
  size_t action = agent.SelectAction(world.GetGrid());
  CHECK(action != 0);
}

TEST_CASE("LearningExplorerAgent visit count for unvisited cell is zero",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  // Check visit count at a cell the agent has never been to
  CHECK(agent.GetVisitedCellCount(world.GetGrid(), WorldPosition{15, 7}) == 0);
  CHECK(agent.GetVisitedCellCount(world.GetGrid(), WorldPosition{1, 5}) == 0);

  // Wall cells should also return 0 visits
  CHECK(agent.GetVisitedCellCount(world.GetGrid(), WorldPosition{0, 0}) == 0);
}

TEST_CASE("LearningExplorerAgent does not move into wall after many steps at boundary",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  // Place near grid edge
  agent.SetLocation(WorldPosition{1, 1});

  const size_t wall_id = world.GetGrid().GetCellTypeID("wall");

  for (int step = 0; step < 50; ++step) {
    size_t action = agent.SelectAction(world.GetGrid());
    int result = world.DoAction(agent, action);
    agent.SetActionResult(result);

    WorldPosition pos = agent.GetLocation().AsWorldPosition();
    REQUIRE(world.GetGrid().IsValid(pos));
    REQUIRE(world.GetGrid()[pos] != wall_id);
  }
}

// ============================================================
//  Initialize edge cases
// ============================================================

TEST_CASE("LearningExplorerAgent Initialize returns true only when all actions exist",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  CHECK(agent.Initialize());

  // Non-existent actions should return false
  CHECK_FALSE(agent.HasAction("jump"));
  CHECK_FALSE(agent.HasAction(""));
  CHECK_FALSE(agent.HasAction("fly"));
}

TEST_CASE("LearningExplorerAgent Initialize can be called multiple times safely",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  CHECK(agent.Initialize());
  CHECK(agent.Initialize());
  CHECK(agent.Initialize());
}

// ============================================================
//  SetActionResult edge cases
// ============================================================

TEST_CASE("LearningExplorerAgent SetActionResult with success updates correctly",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  WorldPosition before = agent.GetLocation().AsWorldPosition();

  size_t action = agent.SelectAction(world.GetGrid());
  int result = world.DoAction(agent, action);
  agent.SetActionResult(result);

  CHECK(result == 1);
  CHECK_FALSE(agent.GetLocation().AsWorldPosition() == before);
}

TEST_CASE("LearningExplorerAgent SetActionResult with failure preserves position",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{1, 1});

  WorldPosition before = agent.GetLocation().AsWorldPosition();

  // Force a move into a wall
  int result = world.DoAction(agent, 1); // MOVE_UP into wall row
  agent.SetActionResult(result);

  CHECK(result == 0);
  CHECK(agent.GetLocation().AsWorldPosition() == before);
}

TEST_CASE("LearningExplorerAgent alternating success and failure results",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  for (int i = 0; i < 20; ++i) {
    size_t action = agent.SelectAction(world.GetGrid());
    int result = world.DoAction(agent, action);
    agent.SetActionResult(result);

    WorldPosition pos = agent.GetLocation().AsWorldPosition();
    CHECK(world.GetGrid().IsValid(pos));
  }

  // Agent should still be functional
  size_t action = agent.SelectAction(world.GetGrid());
  CHECK(action != 0);
}

// ============================================================
//  Getter coverage
// ============================================================

TEST_CASE("LearningExplorerAgent GetName returns correct name",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("TestExplorer");
  agent.SetLocation(WorldPosition{6, 1});
  CHECK(agent.GetName() == "TestExplorer");
}

TEST_CASE("LearningExplorerAgent GetID returns sequential IDs",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &first = world.AddAgent<LearningExplorerAgent>("First");
  first.SetLocation(WorldPosition{6, 1});

  auto &second = world.AddAgent<LearningExplorerAgent>("Second");
  second.SetLocation(WorldPosition{10, 7});

  CHECK(first.GetID() == 0);
  CHECK(second.GetID() == 1);
}

TEST_CASE("LearningExplorerAgent GetLocation returns set position",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  WorldPosition pos = agent.GetLocation().AsWorldPosition();
  CHECK(pos.CellX() == 6);
  CHECK(pos.CellY() == 1);
}

TEST_CASE("LearningExplorerAgent SetSymbol and GetSymbol",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  agent.SetSymbol('E');
  CHECK(agent.GetSymbol() == 'E');

  agent.SetSymbol('X');
  CHECK(agent.GetSymbol() == 'X');
}

TEST_CASE("LearningExplorerAgent GetActionSuccessCount returns zero for all actions",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  // Run some actions first
  for (int i = 0; i < 10; ++i) {
    size_t action = agent.SelectAction(world.GetGrid());
    int result = world.DoAction(agent, action);
    agent.SetActionResult(result);
  }

  // Success/failure counts are stubs that always return 0
  CHECK(agent.GetActionSuccessCount(0) == 0);
  CHECK(agent.GetActionSuccessCount(1) == 0);
  CHECK(agent.GetActionSuccessCount(2) == 0);
  CHECK(agent.GetActionSuccessCount(3) == 0);
  CHECK(agent.GetActionSuccessCount(4) == 0);
  CHECK(agent.GetActionSuccessCount(999) == 0);
}

TEST_CASE("LearningExplorerAgent GetActionFailureCount returns zero for all actions",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  for (int i = 0; i < 10; ++i) {
    size_t action = agent.SelectAction(world.GetGrid());
    int result = world.DoAction(agent, action);
    agent.SetActionResult(result);
  }

  CHECK(agent.GetActionFailureCount(0) == 0);
  CHECK(agent.GetActionFailureCount(1) == 0);
  CHECK(agent.GetActionFailureCount(999) == 0);
}

TEST_CASE("LearningExplorerAgent GetLastAction returns 0 before any action",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  CHECK(agent.GetLastAction() == 0);
}

TEST_CASE("LearningExplorerAgent GetLastAction updates after each SelectAction",
          "[LearningExplorerAgent]") {
  MazeWorld world;
  auto &agent = world.AddAgent<LearningExplorerAgent>("Explorer");
  agent.SetLocation(WorldPosition{6, 1});

  size_t a1 = agent.SelectAction(world.GetGrid());
  CHECK(agent.GetLastAction() == a1);

  world.DoAction(agent, a1);
  agent.SetActionResult(1);

  size_t a2 = agent.SelectAction(world.GetGrid());
  CHECK(agent.GetLastAction() == a2);
}
