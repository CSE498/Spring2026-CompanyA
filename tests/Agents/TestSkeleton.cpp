/**
 * @file TestSkeleton.cpp
 * @author Group 2
 * @brief Tests for skeleton enemy creation through AgentFactory.
 */


#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/AgentFactory.hpp"
#include "../../source/core/WorldBase.hpp"

namespace cse498
{
    class TestWorld : public WorldBase
    {
    public:
        TestWorld() : WorldBase() {}
        ~TestWorld() override = default;

        int DoAction([[maybe_unused]] AgentBase& agent,
                     [[maybe_unused]] size_t action_id) override
        {
            return 0;
        }
    };
}

using namespace cse498;

TEST_CASE("CreateEnemySkeleton by name returns a valid enemy", "[Skeleton][factory]")
{
    TestWorld world;

    auto skeleton = AgentFactory::CreateEnemySkeleton("Bones", world);

    REQUIRE(skeleton != nullptr);
    REQUIRE(skeleton->GetName() == "Bones");
    REQUIRE(skeleton->IsAlive());
    REQUIRE(skeleton->GetHealth() == Approx(100.0));
    REQUIRE(skeleton->GetMaxHealth() == Approx(100.0));
    REQUIRE(skeleton->GetAttackRange() == Approx(3.0));

    // Player is created first by WorldBase, so next created agent should be id 1
    REQUIRE(skeleton->GetID() == 1);

    // Name-only overload defaults spawn to (0, 0)
    REQUIRE(skeleton->GetLocation().AsWorldPosition() == WorldPosition(0, 0));
}

TEST_CASE("CreateEnemySkeleton from AgentDefinition applies definition and spawn", "[Skeleton][factory][definition]")
{
    TestWorld world;

    AgentDefinition def;
    def.name = "EliteSkeleton";
    def.hp = 150.0;
    def.atk = 25.0;

    const WorldPosition spawn(4, 7);

    auto skeleton = AgentFactory::CreateEnemySkeleton(def, world, spawn);

    REQUIRE(skeleton != nullptr);
    REQUIRE(skeleton->GetName() == "EliteSkeleton");
    REQUIRE(skeleton->GetHealth() == Approx(150.0));
    REQUIRE(skeleton->GetMaxHealth() == Approx(150.0));
    REQUIRE(skeleton->GetLocation().AsWorldPosition() == spawn);
    REQUIRE(skeleton->IsAlive());
}

TEST_CASE("Skeleton enemy gold drop can only be claimed once until reset", "[Skeleton][gold]")
{
    TestWorld world;

    auto skeleton = AgentFactory::CreateEnemySkeleton("Bones", world);
    REQUIRE(skeleton != nullptr);

    REQUIRE(skeleton->GetGoldDrop() == 5);

    skeleton->SetGoldDrop(12);
    REQUIRE(skeleton->GetGoldDrop() == 12);

    REQUIRE(skeleton->ClaimGoldDrop() == 12);
    REQUIRE(skeleton->ClaimGoldDrop() == 0);

    skeleton->ResetGoldDropClaim();
    REQUIRE(skeleton->ClaimGoldDrop() == 12);
}

TEST_CASE("Skeleton enemy can die through inherited AgentBase damage handling", "[Skeleton][damage]")
{
    TestWorld world;

    auto skeleton = AgentFactory::CreateEnemySkeleton("Bones", world);
    REQUIRE(skeleton != nullptr);
    REQUIRE(skeleton->IsAlive());

    skeleton->TakeDamage(25.0);
    REQUIRE(skeleton->IsAlive());
    REQUIRE(skeleton->GetHealth() == Approx(75.0));

    skeleton->TakeDamage(100.0);
    REQUIRE_FALSE(skeleton->IsAlive());
    REQUIRE(skeleton->GetHealth() == Approx(0.0));
}

