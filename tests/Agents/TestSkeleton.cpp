/**
 * @file TestSkeleton.cpp
 * @author Group 2
 * @brief Tests for skeleton enemy creation and behavior through AgentFactory.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/Classic/AgentDefinition.hpp"
#include "../../source/Agents/Classic/AgentFactory.hpp"
#include "../../source/Worlds/DemoG2/WorldActions.hpp"
#include "../../source/core/WorldBase.hpp"
#include "../../source/tools/DamageCalculator.hpp"

namespace cse498 {
class SkeletonTestWorld : public WorldBase {
private:
    size_t mFloorId = 0;
    size_t mWallId = 0;
    size_t mLastActorId = 0;
    size_t mLastActionId = 0;

    void ConfigAgent(AgentBase& agent) override {
        agent.AddAction(WorldActions::REMAIN_STILL_STRING, WorldActions::REMAIN_STILL);
        agent.AddAction(WorldActions::MOVE_UP_STRING, WorldActions::MOVE_UP);
        agent.AddAction(WorldActions::MOVE_DOWN_STRING, WorldActions::MOVE_DOWN);
        agent.AddAction(WorldActions::MOVE_LEFT_STRING, WorldActions::MOVE_LEFT);
        agent.AddAction(WorldActions::MOVE_RIGHT_STRING, WorldActions::MOVE_RIGHT);
        agent.AddAction(WorldActions::INTERACT_STRING, WorldActions::INTERACT);
        agent.AddAction(WorldActions::QUIT_STRING, WorldActions::QUIT);
    }

public:
    SkeletonTestWorld() : WorldBase() {

        // KAREN: Create the player here to avoid interfering with other groups' demos (temp fix)
        auto p = std::make_unique<PlayerAgent>(GetNextAgentId(), "Player", *this);
        AddAgent(std::move(p));
        mPlayer = dynamic_cast<PlayerAgent*>(agent_set[0].get());
        assert(mPlayer);


        mFloorId = main_grid.AddCellType("floor", "Walkable floor", ' ');
        mWallId = main_grid.AddCellType("wall", "Blocking wall", '#');

        // SkeletonTestWorld::ConfigAgent(*GetPlayer());
        GetPlayer()->SetStats(AgentStats(40.0, 7.0, 2.0, 1, 0));
        GetPlayer()->SetLocation(WorldPosition(0, 0));
        main_grid.Load(std::vector<std::string>{
                "#######################",
                "#                     #", // (1,1) --> (9, 1)
                "#                 ### #",
                "#             #  #  # #",
                "# #     #  #  #  #  # #",
                "#          #     #    #",
                "##### ############  # #",
                "#                    ##",
                "#                    ##",
                "#  ####################",
                "#######################"});
    }

    int DoAction(AgentBase& agent, size_t action_id) override {
        mLastActorId = agent.GetID();
        mLastActionId = action_id;

        WorldPosition next = agent.GetLocation().AsWorldPosition();
        switch (action_id) {
            case WorldActions::MOVE_UP:
                next = next.GetOffset(0, -1);
                break;
            case WorldActions::MOVE_DOWN:
                next = next.GetOffset(0, 1);
                break;
            case WorldActions::MOVE_LEFT:
                next = next.GetOffset(-1, 0);
                break;
            case WorldActions::MOVE_RIGHT:
                next = next.GetOffset(1, 0);
                break;
            case WorldActions::REMAIN_STILL:
            case WorldActions::INTERACT:
            case WorldActions::QUIT:
                return 1;
            default:
                return 0;
        }

        if (!main_grid.IsWalkable(next)) {
            return 0;
        }

        agent.SetLocation(next);
        return 1;
    }

    void SetPlayerPosition(const WorldPosition& pos) { GetPlayer()->SetLocation(pos); }

    void SetWall(const WorldPosition& pos) { main_grid[pos] = mWallId; }

    void RunNonPlayerAgents() {
        for (const auto& agent_ptr: agent_set) {
            if (agent_ptr.get() == mPlayer) {
                continue;
            }

            const size_t action_id = agent_ptr->SelectAction(main_grid);
            const int result = DoAction(*agent_ptr, action_id);
            agent_ptr->SetActionResult(result);
        }
    }

    [[nodiscard]] size_t GetLastActorId() const { return mLastActorId; }
    [[nodiscard]] size_t GetLastActionId() const { return mLastActionId; }
};
} // namespace cse498

using namespace cse498;

TEST_CASE("Skeleton factory applies definition, stats, actions, and spawn data", "[Skeleton][factory]") {
    SkeletonTestWorld world;
    const AgentDefinition def("EliteSkeleton", 5, {4, 7});

    auto skeleton = AgentFactory::CreateEnemySkeleton(def, world);

    REQUIRE(skeleton != nullptr);
    REQUIRE(skeleton->GetName() == "EliteSkeleton");
    REQUIRE(skeleton->GetID() == 1);
    REQUIRE(skeleton->GetLocation().AsWorldPosition() == WorldPosition(4, 7));
    REQUIRE(skeleton->GetCurrentHealth() == Approx(200.0));
    REQUIRE(skeleton->GetMaxHealth() == Approx(200.0));
    REQUIRE(skeleton->GetAtk() == Approx(15.0));
    REQUIRE(skeleton->GetDef() == Approx(10.0));
    REQUIRE(skeleton->GetAtkRange() == 4);
    REQUIRE(skeleton->GetLevel() == 5);
    REQUIRE(skeleton->IsAlive());

    world.AddAgent(std::move(skeleton));
    auto* stored = dynamic_cast<Enemy*>(world.TryGetAgent(1));
    REQUIRE(stored != nullptr);
    REQUIRE(stored->HasAction(WorldActions::MOVE_UP_STRING));
    REQUIRE(stored->HasAction(WorldActions::MOVE_DOWN_STRING));
    REQUIRE(stored->HasAction(WorldActions::MOVE_LEFT_STRING));
    REQUIRE(stored->HasAction(WorldActions::MOVE_RIGHT_STRING));
    REQUIRE(stored->HasAction(WorldActions::REMAIN_STILL_STRING));
    REQUIRE(stored->HasAction(WorldActions::INTERACT_STRING));
}
