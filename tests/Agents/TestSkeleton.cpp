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

namespace cse498
{
    class SkeletonTestWorld : public WorldBase
    {
    private:
        size_t mFloorId = 0;
        size_t mWallId = 0;
        size_t mLastActorId = 0;
        size_t mLastActionId = 0;

        void ConfigAgent(AgentBase &agent) override
        {
            agent.AddAction(WorldActions::REMAIN_STILL_STRING, WorldActions::REMAIN_STILL);
            agent.AddAction(WorldActions::MOVE_UP_STRING, WorldActions::MOVE_UP);
            agent.AddAction(WorldActions::MOVE_DOWN_STRING, WorldActions::MOVE_DOWN);
            agent.AddAction(WorldActions::MOVE_LEFT_STRING, WorldActions::MOVE_LEFT);
            agent.AddAction(WorldActions::MOVE_RIGHT_STRING, WorldActions::MOVE_RIGHT);
            agent.AddAction(WorldActions::INTERACT_STRING, WorldActions::INTERACT);
            agent.AddAction(WorldActions::QUIT_STRING, WorldActions::QUIT);
        }

    public:
        SkeletonTestWorld()
            : WorldBase()
        {
            mFloorId = main_grid.AddCellType("floor", "Walkable floor", ' ');
            mWallId = main_grid.AddCellType("wall", "Blocking wall", '#');

            SkeletonTestWorld::ConfigAgent(*GetPlayer());
            GetPlayer()->SetStats(AgentStats(40.0, 7.0, 2.0, 1, 0));
            GetPlayer()->SetLocation(WorldPosition(0, 0));
            main_grid.Load(std::vector<std::string>{
            "#######################",
            "#                     #", // (1,1) --> (9, 1)
            "#                 ### #",
            "#             #  #  # #",
            "# #     #  #  #  #  # #",
            "#          #     #    #",
            "##################  # #",
            "#                    ##",
            "#                    ##",
            "#  ####################",
            "#######################"
            });
        }

        int DoAction(AgentBase& agent, size_t action_id) override
        {
            mLastActorId = agent.GetID();
            mLastActionId = action_id;

            WorldPosition next = agent.GetLocation().AsWorldPosition();
            switch (action_id)
            {
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

            if (!main_grid.IsWalkable(next))
            {
                return 0;
            }

            agent.SetLocation(next);
            return 1;
        }

        void SetPlayerPosition(const WorldPosition& pos)
        {
            GetPlayer()->SetLocation(pos);
        }

        void SetWall(const WorldPosition& pos)
        {
            main_grid[pos] = mWallId;
        }

        void RunNonPlayerAgents()
        {
            for (const auto& agent_ptr : agent_set)
            {
                if (agent_ptr.get() == mPlayer)
                {
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
}

using namespace cse498;

TEST_CASE("Skeleton factory applies definition, stats, actions, and spawn data", "[Skeleton][factory]")
{
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

TEST_CASE("Skeleton chases the player by selecting and executing a movement action", "[Skeleton][movement]")
{
    SkeletonTestWorld world;
    world.SetPlayerPosition({4, 1});

    auto skeleton = AgentFactory::CreateEnemySkeleton(AgentDefinition("Bones", 0, {1, 1}), world);
    REQUIRE(skeleton != nullptr);
    auto& stored = world.AddAgent(std::move(skeleton));

    world.RunNonPlayerAgents();

    REQUIRE(world.GetLastActorId() == stored.GetID());
    REQUIRE(world.GetLastActionId() == WorldActions::MOVE_RIGHT);
    REQUIRE(stored.GetActionResult() == 1);
    REQUIRE(stored.GetLocation().AsWorldPosition() == WorldPosition(2, 1));
    REQUIRE(world.GetPlayer()->GetCurrentHealth() == Approx(40.0));
}

TEST_CASE("Skeleton attacks the player instead of moving when the player is in range", "[Skeleton][combat]")
{
    SkeletonTestWorld world;
    world.SetPlayerPosition({3, 1});

    auto skeleton = AgentFactory::CreateEnemySkeleton(AgentDefinition("Bones", 0, {1, 1}), world);
    REQUIRE(skeleton != nullptr);

    const double before = world.GetPlayer()->GetCurrentHealth();
    const double expectedDamage = DamageCalculator::Calculate(skeleton->GetStats(), world.GetPlayer()->GetStats());

    REQUIRE(skeleton->SelectAction(world.GetGrid()) == WorldActions::REMAIN_STILL);
    REQUIRE(world.GetPlayer()->GetCurrentHealth() == Approx(before - expectedDamage));
    REQUIRE(world.GetPlayer()->IsAlive());
}

TEST_CASE("Skeleton chases around combat range only when line of sight is blocked", "[Skeleton][movement][los]")
{
    SkeletonTestWorld world;
    world.SetPlayerPosition({3, 1});
    world.SetWall({2, 1});

    auto skeleton = AgentFactory::CreateEnemySkeleton(AgentDefinition("Bones", 0, {1, 1}), world);
    REQUIRE(skeleton != nullptr);
    auto& stored = world.AddAgent(std::move(skeleton));

    world.RunNonPlayerAgents();

    REQUIRE(world.GetPlayer()->GetCurrentHealth() == Approx(40.0));
    REQUIRE(world.GetLastActionId() != WorldActions::REMAIN_STILL);
    REQUIRE(stored.GetLocation().AsWorldPosition() != WorldPosition(1, 1));
}

TEST_CASE("Skeleton stays put when no path to the player exists", "[Skeleton][movement][blocked]")
{
    SkeletonTestWorld world;
    world.SetPlayerPosition({5, 5});

    auto skeleton = AgentFactory::CreateEnemySkeleton(AgentDefinition("Bones", 0, {1, 1}), world);
    REQUIRE(skeleton != nullptr);
    auto& stored = world.AddAgent(std::move(skeleton));

    world.SetWall({0, 0});
    world.SetWall({0, 1});
    world.SetWall({0, 2});
    world.SetWall({1, 0});
    world.SetWall({1, 2});
    world.SetWall({2, 0});
    world.SetWall({2, 1});
    world.SetWall({2, 2});

    const size_t selectedAction = stored.SelectAction(world.GetGrid());

    REQUIRE(selectedAction == WorldActions::REMAIN_STILL);
    REQUIRE(stored.GetLocation().AsWorldPosition() == WorldPosition(1, 1));
}

TEST_CASE("Skeleton gold and lifetime interactions behave consistently", "[Skeleton][gold][damage]")
{
    SkeletonTestWorld world;
    auto skeleton = AgentFactory::CreateEnemySkeleton(AgentDefinition("Bones", 0, {3, 4}), world);
    REQUIRE(skeleton != nullptr);
    auto& stored = world.AddAgent(std::move(skeleton));
    const size_t storedId = stored.GetID();

    REQUIRE(stored.GetGoldDrop() == 5);
    stored.SetGoldDrop(12);
    REQUIRE(stored.GetGoldDrop() == 12);
    REQUIRE(stored.ClaimGoldDrop() == 12);
    REQUIRE(stored.ClaimGoldDrop() == 0);
    stored.ResetGoldDropClaim();
    REQUIRE(stored.ClaimGoldDrop() == 12);

    stored.TakeDamage(25.0);
    REQUIRE(stored.IsAlive());
    REQUIRE(stored.GetCurrentHealth() == Approx(75.0));

    stored.TakeDamage(100.0);
    REQUIRE_FALSE(stored.IsAlive());
    REQUIRE(stored.GetCurrentHealth() == Approx(0.0));

    world.RemoveDeadAgents();
    REQUIRE_FALSE(world.HasAgent(storedId));
    REQUIRE(world.GetPlayer() != nullptr);
}
