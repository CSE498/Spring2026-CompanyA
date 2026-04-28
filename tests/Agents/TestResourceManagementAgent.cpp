/**
 * @file TestResourceManagementAgent.cpp
 * @brief Unit tests for interactive-world resource management interactions.
 */

// Used codex for test generation

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/Classic/ResourceManagementAgent.hpp"
#include "../../source/Interfaces/TrashInterface.hpp"
#include "../../source/Worlds/Hub/InteractiveWorld.hpp"

using namespace cse498;

namespace {

class DummyInteractable : public AgentBase {
public:
    DummyInteractable(size_t id, const std::string& name, const WorldBase& world) : AgentBase(id, name, world) {}

    [[nodiscard]] size_t SelectAction(const WorldGrid&) override { return 0; }

    bool Interact() override {
        interacted = true;
        return true;
    }

    bool interacted = false;
};

} // namespace

TEST_CASE("ResourceManagementAgent upgrades managed buildings independently", "[ResourceManagementAgent][upgrade]") {
    InteractiveWorld world;

    Building& lumberYard = world.AddAgent<Building>("Lumber Yard");
    lumberYard.AddUpgrade(ItemType::Wood, 15);
    world.AddBuilding(lumberYard, WorldPosition{4, 4});

    Building& quarry = world.AddAgent<Building>("Quarry");
    quarry.AddUpgrade(ItemType::Stone, 10);
    world.AddBuilding(quarry, WorldPosition{6, 4});

    ResourceManagementAgent& manager = world.AddAgent<ResourceManagementAgent>("Manager");
    manager.SetInventory(world.GetInventoryPtr()).SetManagedBuildings(world.GetBuildings());

    REQUIRE(world.GetInventory().AddItem(ItemType::Wood, 20));
    REQUIRE(world.GetInventory().AddItem(ItemType::Stone, 12));

    std::string message;
    REQUIRE(manager.UpgradeBuilding(1, &message));
    CHECK(message == "Quarry upgraded to level 1.");
    CHECK(quarry.GetCurrentLevel() == 1);
    CHECK(lumberYard.GetCurrentLevel() == 0);
    CHECK(world.GetInventory().GetAmount(ItemType::Stone) == 2);
    CHECK(world.GetInventory().GetAmount(ItemType::Wood) == 20);

    REQUIRE(manager.UpgradeBuilding(0, &message));
    CHECK(message == "Lumber Yard upgraded to level 1.");
    CHECK(lumberYard.GetCurrentLevel() == 1);
    CHECK(world.GetInventory().GetAmount(ItemType::Wood) == 5);
}

TEST_CASE("ResourceManagementAgent sells stored resources for gold", "[ResourceManagementAgent][sell]") {
    InteractiveWorld world;

    ResourceManagementAgent& manager = world.AddAgent<ResourceManagementAgent>("Manager");
    manager.SetInventory(world.GetInventoryPtr());

    REQUIRE(world.GetInventory().AddItem(ItemType::Metal, 7));

    std::string message;
    REQUIRE(manager.SellResource(ItemType::Metal, 4, &message));
    CHECK(message == "Sold 4 Metal for 12 gold.");
    CHECK(manager.GetGold() == 12);
    CHECK(world.GetInventory().GetAmount(ItemType::Metal) == 3);
}

TEST_CASE("ResourceManagementAgent rejects upgrades without enough resources", "[ResourceManagementAgent][upgrade]") {
    InteractiveWorld world;

    Building& mine = world.AddAgent<Building>("Mine");
    mine.AddUpgrade(ItemType::Metal, 25);
    world.AddBuilding(mine, WorldPosition{5, 5});

    ResourceManagementAgent& manager = world.AddAgent<ResourceManagementAgent>("Manager");
    manager.SetInventory(world.GetInventoryPtr()).SetManagedBuildings(world.GetBuildings());

    REQUIRE(world.GetInventory().AddItem(ItemType::Metal, 10));

    std::string message;
    CHECK_FALSE(manager.UpgradeBuilding(0, &message));
    CHECK(message == "Not enough Metal to upgrade Mine.");
    CHECK(mine.GetCurrentLevel() == 0);
    CHECK(world.GetInventory().GetAmount(ItemType::Metal) == 10);
}

TEST_CASE("InteractiveWorld dispatches interface interact actions to adjacent agents", "[InteractiveWorld][interact]") {
    InteractiveWorld world;

    TrashInterface& player = world.AddAgent<TrashInterface>("Interface");
    player.SetPosition(WorldPosition{4, 4});

    DummyInteractable& interactable = world.AddAgent<DummyInteractable>("Interactable");
    interactable.SetPosition(WorldPosition{5, 4});

    REQUIRE(world.DoAction(player, player.GetActionID("interact")) == 1);
    CHECK(interactable.interacted);
}
