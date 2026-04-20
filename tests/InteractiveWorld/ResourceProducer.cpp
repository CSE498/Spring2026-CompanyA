#include "../../source/Worlds/Hub/ResourceProducer.hpp"
#include "../../source/Worlds/Hub/InteractiveWorldInventory.hpp"
#include "catch2/catch.hpp"

#include <string>

using cse498::ItemType;
#include "../../source/Worlds/Hub/InteractiveWorld.hpp"
#include "../../source/Worlds/Hub/ResourceSpawn.hpp"

static cse498::InteractiveWorld world;
using cse498::ItemType;
using cse498::ResourceSpawn;
using cse498::WorldPosition;

bool ApproxEqual(float a, float b, float tolerance = 1e-5f) { return std::abs(a - b) <= tolerance; }

TEST_CASE("Test ResourceProducer Constructor", "[core][InteractiveWorld][ResourceProducer]") {
    cse498::Building& farm = world.AddAgent<cse498::Building>("Farm");
    CHECK(farm.GetName() == "Farm");

	auto spawnPtr = std::make_unique<ResourceSpawn>(
	    world.GetNextAgentId(), "Wood Spawn", world, ItemType::Wood);
	spawnPtr->SetSymbol('l');
	ResourceSpawn& spawn = world.AddAgent(std::move(spawnPtr));
	spawn.SetLocation(WorldPosition{5, 5});

    cse498::InteractiveWorldInventory inv;
    float baseRate = 1.0f;
    cse498::ResourceProducer producer(farm, spawn, ItemType::Wood, baseRate);

    CHECK(producer.GetRate() == baseRate);
}

TEST_CASE("Test ResourceProducer Rate increase", "[core][InteractiveWorld][ResourceProducer]") {
    cse498::Building& farm = world.AddAgent<cse498::Building>("Farm2");
    CHECK(farm.GetName() == "Farm2");
    farm.AddUpgrade(ItemType::Wood, 5);
    farm.AddUpgrade(ItemType::Wood, 5);
    farm.AddUpgrade(ItemType::Wood, 5);
    farm.SetRateModifier(0.25f);

	auto spawnPtr = std::make_unique<ResourceSpawn>(
	    world.GetNextAgentId(), "Wood Spawn", world, ItemType::Wood);
	spawnPtr->SetSymbol('l');
	ResourceSpawn& spawn = world.AddAgent(std::move(spawnPtr));
	spawn.SetLocation(WorldPosition{5, 5});

    cse498::InteractiveWorldInventory inv;
    inv.AddItem(ItemType::Wood, 15);
    float baseRate = 1.0f;
    cse498::ResourceProducer producer(farm, spawn, ItemType::Wood, baseRate);

    CHECK(producer.GetRate() == baseRate);
    ItemType type = ItemType::Wood;

    int inventoryWoodCount = inv.GetAmount(type);
    const auto firstUpgrade = farm.Upgrade(type, inventoryWoodCount);
    REQUIRE(firstUpgrade.has_value());
    CHECK(farm.GetCurrentLevel() == 1);
    producer.CalculateRate();
    CHECK(ApproxEqual(producer.GetRate(), 1.25f));

    const auto secondUpgrade = farm.Upgrade(type, inventoryWoodCount);
    REQUIRE(secondUpgrade.has_value());
    CHECK(farm.GetCurrentLevel() == 2);
    producer.CalculateRate();
    CHECK(ApproxEqual(producer.GetRate(), 1.5f));

    const auto thirdUpgrade = farm.Upgrade(type, inventoryWoodCount);
    REQUIRE(thirdUpgrade.has_value());
    CHECK(farm.GetCurrentLevel() == 3);
    producer.CalculateRate();
    CHECK(ApproxEqual(producer.GetRate(), 1.75f));
}
