/**
 * @file TestPlayer.cpp
 * @author lrima
 */

#include "../../source/Agents/Classic/AgentStats.hpp"
#include "../../source/Agents/Classic/PlayerAgent.hpp"
#include "../../source/Agents/Classic/PlayerFeatures/Inventory.hpp"
#include "../../source/core/WorldBase.hpp"
#include "../../source/core/item/ItemWeaponBow.hpp"
#include "../../source/core/item/ItemWeaponToolShovel.hpp"
#include "../../source/core/item/ItemWeaponSword.hpp"
#include "../../source/tools/DamageCalculator.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

namespace cse498 {
class MockWorldBase : public WorldBase {
public:
    MockWorldBase() : WorldBase() {
        // KAREN: Create the player here to avoid interfering with other groups' demos (temp fix)
        auto p = std::make_unique<PlayerAgent>(GetNextAgentId(), "Player", *this);
        AddAgent(std::move(p));
        mPlayer = dynamic_cast<PlayerAgent*>(agent_set[0].get());
        assert(mPlayer);
    }
    ~MockWorldBase() override = default;
    int DoAction([[maybe_unused]] AgentBase& agent, [[maybe_unused]] size_t action_id) override { return 0; }
};

} // namespace cse498


using cse498::PlayerAgent;
using cse498::AgentStats;
using cse498::DamageCalculator;
using cse498::Inventory;
using cse498::ItemWeaponBow;
using cse498::ItemWeaponSword;
using cse498::ItemWeaponToolShovel;


TEST_CASE("Main Player", "[Inventory]") {
    // Ensure that it is accessible and there is a way to access it
    cse498::MockWorldBase world;
    auto* player = world.GetPlayer(); // shoudln't ever be null
    CHECK(player);
    auto& inv = player->GetInventory();
    CHECK(inv.GetHand() == nullptr);
}

TEST_CASE("Player sword in hand adds damage and sets melee range", "[Player][Weapon]") {
    cse498::MockWorldBase world;
    auto* player = world.GetPlayer();
    REQUIRE(player);

    player->SetStats(AgentStats(100.0, 10.0, 2.0, 3.0, 1));
    auto& inv = player->GetInventory();
    inv.AddItem<ItemWeaponSword>(1, 1, "Test Sword", "", 5, world);
    inv.SwapSlots(0, Inventory::HOTBAR_SIZE);

    CHECK(player->GetAtk() == Approx(15.0));
    CHECK(player->GetAtkRange() == Approx(1.0));
    CHECK(player->GetDef() == Approx(2.0));
    CHECK(player->GetCurrentHealth() == Approx(100.0));
}

TEST_CASE("Player bow in hand uses ranged stats", "[Player][Weapon]") {
    cse498::MockWorldBase world;
    auto* player = world.GetPlayer();
    player->SetStats(AgentStats(50.0, 8.0, 1.0, 2.0, 2));
    auto& inv = player->GetInventory();
    inv.AddItem<ItemWeaponBow>(1, 2, "Test Bow", "", 8, world);
    inv.SwapSlots(0, Inventory::HOTBAR_SIZE);

    CHECK(player->GetAtk() == Approx(11.0));
    CHECK(player->GetAtkRange() == Approx(5.0));
}

TEST_CASE("Player empty hotbar slot uses base combat stats", "[Player][Weapon]") {
    cse498::MockWorldBase world;
    auto* player = world.GetPlayer();
    player->SetStats(AgentStats(100.0, 10.0, 2.0, 3.0, 1));
    auto& inv = player->GetInventory();
    inv.AddItem<ItemWeaponSword>(1, 1, "Test Sword", "", 5, world);
    inv.SwapSlots(0, Inventory::HOTBAR_SIZE);
    inv.HotBarIndexInc();

    CHECK(player->GetAtk() == Approx(10.0));
    CHECK(player->GetAtkRange() == Approx(3.0));
}

TEST_CASE("Weapon tool in hand does not modify combat stats", "[Player][Weapon]") {
    cse498::MockWorldBase world;
    auto* player = world.GetPlayer();
    player->SetStats(AgentStats(100.0, 10.0, 2.0, 3.0, 1));
    auto& inv = player->GetInventory();
    inv.AddItem<ItemWeaponToolShovel>(1, 3, "Test Shovel", "", 2, world);
    inv.SwapSlots(0, Inventory::HOTBAR_SIZE);

    CHECK(player->GetAtk() == Approx(10.0));
    CHECK(player->GetAtkRange() == Approx(3.0));
}

TEST_CASE("SetStats recomputes weapon bonuses from new base", "[Player][Weapon]") {
    cse498::MockWorldBase world;
    auto* player = world.GetPlayer();
    player->SetStats(AgentStats(100.0, 10.0, 2.0, 3.0, 1));
    auto& inv = player->GetInventory();
    inv.AddItem<ItemWeaponSword>(1, 1, "Test Sword", "", 5, world);
    inv.SwapSlots(0, Inventory::HOTBAR_SIZE);
    CHECK(player->GetAtk() == Approx(15.0));

    player->SetStats(AgentStats(100.0, 20.0, 2.0, 4.0, 1));
    CHECK(player->GetAtk() == Approx(25.0));
    CHECK(player->GetAtkRange() == Approx(1.0));
}

TEST_CASE("DamageCalculator uses buffed player attack from equipped weapon", "[Player][Weapon]") {
    cse498::MockWorldBase world;
    auto* player = world.GetPlayer();
    player->SetStats(AgentStats(100.0, 10.0, 2.0, 3.0, 1));
    auto& inv = player->GetInventory();
    inv.AddItem<ItemWeaponSword>(1, 1, "Test Sword", "", 5, world);
    inv.SwapSlots(0, Inventory::HOTBAR_SIZE);

    AgentStats defender(50.0, 0.0, 5.0, 1.0, 1);
    const double dmg = DamageCalculator::Calculate(player->GetStats(), defender);
    CHECK(dmg == Approx(10.0));
}

TEST_CASE("ClearInventory clears hand and restores base combat stats", "[Player][Weapon]") {
    cse498::MockWorldBase world;
    auto* player = world.GetPlayer();
    player->SetStats(AgentStats(100.0, 10.0, 2.0, 3.0, 1));
    auto& inv = player->GetInventory();
    inv.AddItem<ItemWeaponSword>(1, 1, "Test Sword", "", 5, world);
    inv.SwapSlots(0, Inventory::HOTBAR_SIZE);
    CHECK(player->GetAtk() == Approx(15.0));

    inv.ClearInventory();
    CHECK(inv.GetHand() == nullptr);
    CHECK(player->GetAtk() == Approx(10.0));
    CHECK(player->GetAtkRange() == Approx(3.0));
}
