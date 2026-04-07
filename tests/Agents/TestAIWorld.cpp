/**
 * @file TestAIWorld.cpp
 * @brief Catch2 unit tests for AIWorld, LootItem, and world combat/loot actions.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/LearningExplorerAgent.hpp"
#include "../../source/Agents/TrailblazerAgent.hpp"
#include "../../source/Worlds/AIWorld.hpp"

#include <sstream>

using namespace cse498;

namespace {

    struct AIWorldHarness : AIWorld {
    public:
        using AIWorld::mAgentState;
        using AIWorld::mCurrentTurn;
        using AIWorld::mEnemies;
        using AIWorld::mMaxTurns;

        void InvokeHandleEnemyAt(size_t index) { HandleEnemyTurn(mEnemies.at(index)); }

        bool InvokeWaitForStep() { return WaitForStep(); }

        std::string InvokeActionName(size_t id) const { return ActionName(id); }

        void InvokePrintWorldState(const std::string &header) const { PrintWorldState(header); }

        bool InvokeIsWalkable(WorldPosition pos) const { return IsWalkable(pos); }

        bool InvokeIsEnemyAt(WorldPosition pos, size_t *enemy_index = nullptr) const {
            return IsEnemyAt(pos, enemy_index);
        }

        bool InvokeIsAgentAt(WorldPosition pos, size_t skip_agent_id = static_cast<size_t>(-1)) const {
            return IsAgentAt(pos, skip_agent_id);
        }

        std::optional<size_t> InvokeItemOnFloor(WorldPosition pos) const { return ItemOnFloor(pos); }
    };

} // namespace

TEST_CASE("LootItem weapon properties and Consume", "[AIWorld][LootItem]") {
    AIWorldHarness world;
    LootItem item(0, "Blade", world, LootItem::ItemKind::Weapon, 4);
    CHECK(item.GetKind() == LootItem::ItemKind::Weapon);
    CHECK(item.GetPower() == 4);
    CHECK_FALSE(item.IsConsumed());
    item.Consume();
    CHECK(item.IsConsumed());
}

TEST_CASE("LootItem heal kind", "[AIWorld][LootItem]") {
    AIWorldHarness world;
    LootItem item(1, "Vial", world, LootItem::ItemKind::Heal, 2);
    CHECK(item.GetKind() == LootItem::ItemKind::Heal);
    CHECK(item.GetPower() == 2);
}

TEST_CASE("AIWorld GetFloorItems lists default loot", "[AIWorld]") {
    AIWorldHarness world;
    const auto items = world.GetFloorItems();
    CHECK_FALSE(items.empty());
    bool saw_weapon = false;
    bool saw_heal = false;
    for (const auto &v: items) {
        if (v.mKind == LootItem::ItemKind::Weapon) {
            saw_weapon = true;
        }
        if (v.mKind == LootItem::ItemKind::Heal) {
            saw_heal = true;
        }
    }
    CHECK(saw_weapon);
    CHECK(saw_heal);
}

TEST_CASE("AIWorld InvokeActionName covers known ids and default", "[AIWorld]") {
    AIWorldHarness world;
    CHECK(world.InvokeActionName(0) == "remain");
    CHECK(world.InvokeActionName(1) == "move_up");
    CHECK(world.InvokeActionName(2) == "move_down");
    CHECK(world.InvokeActionName(3) == "move_left");
    CHECK(world.InvokeActionName(4) == "move_right");
    CHECK(world.InvokeActionName(5) == "pickup");
    CHECK(world.InvokeActionName(6) == "use_heal");
    CHECK(world.InvokeActionName(7) == "attack_up");
    CHECK(world.InvokeActionName(8) == "attack_down");
    CHECK(world.InvokeActionName(9) == "attack_left");
    CHECK(world.InvokeActionName(10) == "attack_right");
    CHECK(world.InvokeActionName(99999) == "unknown");
}

TEST_CASE("AIWorld IsWalkable rejects wall and out of bounds", "[AIWorld]") {
    AIWorldHarness world;
    CHECK_FALSE(world.InvokeIsWalkable(WorldPosition{0, 0}));
    CHECK(world.InvokeIsWalkable(WorldPosition{1, 1}));
}

TEST_CASE("AIWorld IsEnemyAtPosition and index out-parameter", "[AIWorld]") {
    AIWorldHarness world;
    size_t idx = 999;
    CHECK(world.InvokeIsEnemyAt(WorldPosition{5, 5}, &idx));
    CHECK(idx == 0);
    CHECK(world.IsEnemyAtPosition(WorldPosition{5, 5}));
}

TEST_CASE("AIWorld IsAgentAt respects skip id", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    world.SetStepMode(false);
    auto &agent = world.AddAgent<LearningExplorerAgent>("A");
    agent.SetLocation(WorldPosition{10, 7});
    CHECK(world.InvokeIsAgentAt(WorldPosition{10, 7}));
    CHECK_FALSE(world.InvokeIsAgentAt(WorldPosition{10, 7}, agent.GetID()));
}

TEST_CASE("AIWorld ItemOnFloor finds loot at sword tile", "[AIWorld]") {
    AIWorldHarness world;
    const auto id = world.InvokeItemOnFloor(WorldPosition{3, 5});
    REQUIRE(id.has_value());
    const auto *loot = dynamic_cast<const LootItem *>(&world.GetItem(*id));
    REQUIRE(loot != nullptr);
    CHECK(loot->GetKind() == LootItem::ItemKind::Weapon);
}

TEST_CASE("AIWorld DoAction pickup weapon increases attack and score", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{3, 5});
    const auto &before = world.GetAgentState(hero.GetID());
    const int atk_before = before.mAttack;

    CHECK(world.DoAction(hero, hero.GetActionID("pickup")) != 0);
    const auto &after = world.GetAgentState(hero.GetID());
    CHECK(after.mAttack > atk_before);
    CHECK(after.mScore >= 15);
}

TEST_CASE("AIWorld DoAction pickup heal grants charges", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{13, 8});

    CHECK(world.DoAction(hero, hero.GetActionID("pickup")) != 0);
    CHECK(world.GetAgentState(hero.GetID()).mHealCharges > 0);
}

TEST_CASE("AIWorld DoAction use_heal restores HP when damaged", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{10, 7});

    auto &st = world.mAgentState.at(hero.GetID());
    st.mHealCharges = 2;
    st.mHP = 5;
    st.mMaxHP = 12;

    CHECK(world.DoAction(hero, hero.GetActionID("use_heal")) != 0);
    CHECK(world.GetAgentState(hero.GetID()).mHP > 5);
    CHECK(world.GetAgentState(hero.GetID()).mHealCharges == 1);
}

TEST_CASE("AIWorld DoAction use_heal fails with no charges", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{10, 7});
    auto &st = world.mAgentState.at(hero.GetID());
    st.mHealCharges = 0;
    st.mHP = 5;
    CHECK(world.DoAction(hero, hero.GetActionID("use_heal")) == 0);
}

TEST_CASE("AIWorld DoAction use_heal fails at full HP", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{10, 7});
    auto &st = world.mAgentState.at(hero.GetID());
    st.mHealCharges = 3;
    st.mHP = st.mMaxHP;
    CHECK(world.DoAction(hero, hero.GetActionID("use_heal")) == 0);
}

TEST_CASE("AIWorld DoAction attack damages and can kill enemy", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{4, 5});

    const int hp_before = world.GetEnemies()[0].mHP;
    CHECK(world.DoAction(hero, hero.GetActionID("attack_right")) != 0);
    CHECK(world.GetEnemies()[0].mHP < hp_before);

    while (world.GetEnemies()[0].mAlive) {
        REQUIRE(world.DoAction(hero, hero.GetActionID("attack_right")) != 0);
    }
    CHECK_FALSE(world.GetEnemies()[0].mAlive);
    CHECK(world.GetAgentState(hero.GetID()).mScore >= 50);
}

TEST_CASE("AIWorld DoAction attack misses without adjacent enemy", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{10, 7});
    CHECK(world.DoAction(hero, hero.GetActionID("attack_up")) == 0);
}

TEST_CASE("AIWorld DoAction move blocked by enemy cell", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{4, 5});
    CHECK(world.DoAction(hero, hero.GetActionID("right")) == 0);
}

TEST_CASE("AIWorld DoAction move blocked by other agent", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    auto &a = world.AddAgent<LearningExplorerAgent>("A");
    auto &b = world.AddAgent<LearningExplorerAgent>("B");
    a.SetLocation(WorldPosition{10, 7});
    b.SetLocation(WorldPosition{11, 7});
    CHECK(world.DoAction(a, a.GetActionID("right")) == 0);
}

TEST_CASE("AIWorld DoAction no-op when agent HP is zero", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{10, 7});
    world.mAgentState.at(hero.GetID()).mHP = 0;
    CHECK(world.DoAction(hero, hero.GetActionID("up")) == 0);
}

TEST_CASE("AIWorld DoAction pickup fails on empty tile", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{10, 7});
    CHECK(world.DoAction(hero, hero.GetActionID("pickup")) == 0);
}

TEST_CASE("AIWorld HandleEnemyTurn attacks adjacent agent", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{5, 4});
    const int hp_before = world.mAgentState.at(hero.GetID()).mHP;

    world.InvokeHandleEnemyAt(0);
    CHECK(world.mAgentState.at(hero.GetID()).mHP < hp_before);
}

TEST_CASE("AIWorld HandleEnemyTurn no-op when no agents registered", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    const WorldPosition pos_before = world.mEnemies[0].mPosition;
    world.InvokeHandleEnemyAt(0);
    CHECK(world.mEnemies[0].mPosition == pos_before);
}

TEST_CASE("AIWorld HandleEnemyTurn skips dead enemy", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{5, 4});
    const int hp0 = world.mAgentState.at(hero.GetID()).mHP;

    world.mEnemies[0].mAlive = false;
    world.InvokeHandleEnemyAt(0);
    CHECK(world.mAgentState.at(hero.GetID()).mHP == hp0);
}

TEST_CASE("AIWorld InvokePrintWorldState quiet when not verbose", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    world.InvokePrintWorldState("quiet");
}

TEST_CASE("AIWorld WaitForStep non-interactive when step mode off", "[AIWorld]") {
    AIWorldHarness world;
    world.SetStepMode(false);
    CHECK(world.InvokeWaitForStep());
}

TEST_CASE("AIWorld WaitForStep quit line returns false", "[AIWorld]") {
    AIWorldHarness world;
    world.SetStepMode(true);
    std::istringstream input("q\n");
    std::streambuf *old = std::cin.rdbuf(input.rdbuf());
    const bool cont = world.InvokeWaitForStep();
    std::cin.rdbuf(old);
    CHECK_FALSE(cont);
}

TEST_CASE("AIWorld RunAgents skips zero-HP agent", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    world.SetStepMode(false);
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{10, 7});
    world.mAgentState.at(hero.GetID()).mHP = 0;

    world.RunAgents();
    CHECK(hero.GetLocation().AsWorldPosition() == WorldPosition{10, 7});
}

TEST_CASE("AIWorld UpdateWorld ends when all enemies dead", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    world.SetStepMode(false);
    world.AddAgent<TrailblazerAgent>("Hero").SetLocation(WorldPosition{10, 7});

    for (auto &e: world.mEnemies) {
        e.mAlive = false;
    }

    world.UpdateWorld();
    CHECK(world.IsRunOver());
}

TEST_CASE("AIWorld UpdateWorld ends when all agents dead", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    world.SetStepMode(false);
    world.AddAgent<TrailblazerAgent>("Hero").SetLocation(WorldPosition{10, 7});
    world.mAgentState.at(0).mHP = 0;

    world.UpdateWorld();
    CHECK(world.IsRunOver());
}

TEST_CASE("AIWorld UpdateWorld ends at turn limit", "[AIWorld]") {
    AIWorldHarness world;
    world.SetVerboseMode(false);
    world.SetStepMode(false);
    world.AddAgent<TrailblazerAgent>("Hero").SetLocation(WorldPosition{10, 7});
    world.mMaxTurns = 1;
    world.mCurrentTurn = 0;

    world.UpdateWorld();
    CHECK(world.IsRunOver());
}

TEST_CASE("AIWorld GetAgentState reflects ConfigAgent defaults", "[AIWorld]") {
    AIWorldHarness world;
    auto &hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{10, 7});
    const auto &st = world.GetAgentState(hero.GetID());
    CHECK(st.mMaxHP == 12);
    CHECK(st.mHP == 12);
    CHECK(st.mAttack == 2);
}
