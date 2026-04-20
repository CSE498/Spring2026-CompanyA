/**
 * @file TestPacingAgent.cpp
 * @brief Unit tests for PacingAgent movement behavior.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/core/WorldBase.hpp"
#include "../../source/core/WorldGrid.hpp"

using cse498::AgentBase;
using cse498::PacingAgent;
using cse498::WorldGrid;
using cse498::WorldPosition;

namespace {

/// Simple test world that just provides movement actions and a walkable grid.
class PacingTestWorld : public cse498::WorldBase {
protected:
    enum ActionType { REMAIN_STILL = 0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };

    size_t floor_id{};
    size_t wall_id{};

    void ConfigAgent(AgentBase& agent) override {
        agent.AddAction("up", MOVE_UP);
        agent.AddAction("down", MOVE_DOWN);
        agent.AddAction("left", MOVE_LEFT);
        agent.AddAction("right", MOVE_RIGHT);
    }

public:
    PacingTestWorld() {
        // KAREN: Create the player here to avoid interfering with other groups' demos (temp fix)
        auto p = std::make_unique<cse498::PlayerAgent>(GetNextAgentId(), "Player", *this);
        AddAgent(std::move(p));
        mPlayer = dynamic_cast<cse498::PlayerAgent*>(agent_set[0].get());
        assert(mPlayer);

        // Simple open 5x5 grid with outer walls.
        floor_id = main_grid.AddCellType("floor", "Floor that agents can walk on.", ' ');
        wall_id = main_grid.AddCellType("wall", "Impenetrable wall.", '#');

        main_grid.Load(std::vector<std::string>{"#####", "#   #", "#   #", "#   #", "#####"});
    }

    int DoAction(AgentBase& agent, size_t action_id) override {
        WorldPosition cur = agent.GetLocation().AsWorldPosition();
        WorldPosition next = cur;
        switch (action_id) {
            case REMAIN_STILL:
                break;
            case MOVE_UP:
                next = cur.Up();
                break;
            case MOVE_DOWN:
                next = cur.Down();
                break;
            case MOVE_LEFT:
                next = cur.Left();
                break;
            case MOVE_RIGHT:
                next = cur.Right();
                break;
            default:
                return 0;
        }

        if (!main_grid.IsValid(next))
            return 0;
        if (main_grid[next] == wall_id)
            return 0;

        agent.SetLocation(next);
        return 1;
    }
};

} // anonymous namespace

TEST_CASE("PacingAgent requires basic movement actions", "[PacingAgent][Initialize]") {
    PacingTestWorld world;
    auto agent_ptr = std::make_unique<PacingAgent>(1, "pacer", world);

    // WorldBase::AddAgent will call ConfigAgent and then Initialize().
    auto& pacer = world.AddAgent(std::move(agent_ptr));

    // Initialize should have succeeded because ConfigAgent added all four actions.
    CHECK(pacer.Initialize());
}

TEST_CASE("PacingAgent vertical pacing toggles on failure", "[PacingAgent][Vertical]") {
    PacingTestWorld world;
    auto agent_ptr = std::make_unique<PacingAgent>(1, "pacer", world);
    auto& pacer = world.AddAgent(std::move(agent_ptr));

    // Start the agent in the middle of the vertical corridor.
    pacer.SetLocation(WorldPosition(2, 2)); // (x=2,y=2) is open floor
    pacer.SetVertical(); // move up/down

    // First move should be down (since reverse == false).
    size_t action_id = pacer.SelectAction(world.GetGrid());
    int result = world.DoAction(pacer, action_id);
    pacer.SetActionResult(result);
    CHECK(result == 1);
    CHECK(pacer.GetLocation().AsWorldPosition().Y() == Approx(3.0));

    // Move down again into the wall; this should fail and flip direction.
    action_id = pacer.SelectAction(world.GetGrid());
    result = world.DoAction(pacer, action_id);
    pacer.SetActionResult(result);
    CHECK(result == 0); // hit bottom wall

    // After a failed move, next SelectAction should go the opposite way (up).
    action_id = pacer.SelectAction(world.GetGrid());
    result = world.DoAction(pacer, action_id);
    pacer.SetActionResult(result);
    CHECK(result == 1);
    CHECK(pacer.GetLocation().AsWorldPosition().Y() == Approx(2.0));
}

TEST_CASE("PacingAgent horizontal pacing toggles on failure", "[PacingAgent][Horizontal]") {
    PacingTestWorld world;
    auto agent_ptr = std::make_unique<PacingAgent>(1, "pacer", world);
    auto& pacer = world.AddAgent(std::move(agent_ptr));

    // Start the agent in the middle of the horizontal corridor.
    pacer.SetLocation(WorldPosition(2, 2));
    pacer.SetHorizontal(); // move left/right

    // First move should be right (reverse == false).
    size_t action_id = pacer.SelectAction(world.GetGrid());
    int result = world.DoAction(pacer, action_id);
    pacer.SetActionResult(result);
    CHECK(result == 1);
    CHECK(pacer.GetLocation().AsWorldPosition().X() == Approx(3.0));

    // Move right again into the wall; this should fail and flip direction.
    action_id = pacer.SelectAction(world.GetGrid());
    result = world.DoAction(pacer, action_id);
    pacer.SetActionResult(result);
    CHECK(result == 0); // hit right wall

    // After a failed move, next SelectAction should go the opposite way (left).
    action_id = pacer.SelectAction(world.GetGrid());
    result = world.DoAction(pacer, action_id);
    pacer.SetActionResult(result);
    CHECK(result == 1);
    CHECK(pacer.GetLocation().AsWorldPosition().X() == Approx(2.0));
}
