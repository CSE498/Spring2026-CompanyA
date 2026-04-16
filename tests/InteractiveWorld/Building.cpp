#include "../../source/Worlds/Hub/Building.hpp"
#include "../../source/Worlds/Hub/InteractiveWorld.hpp"
#include "catch2/catch.hpp"

#include <string>

/// used codex for some testcase generations

static cse498::InteractiveWorld world;

TEST_CASE("Test Building Constructor", "[core][InteractiveWorld][Building]") {
  std::string farmStr = "Farm";
  std::string blacksmithStr = "Blacksmith";
  cse498::Building farm(1, farmStr, world);
  cse498::Building blacksmith(2, blacksmithStr, world);

  CHECK(farm.GetName() == farmStr);
  CHECK(blacksmith.GetName() == blacksmithStr);
}

TEST_CASE("Test Building Setters and Getters",
          "[core][InteractiveWorld][Building]") {
  std::string farmStr = "Farm";
  std::string blacksmithStr = "Blacksmith";
  cse498::Building farm(1, farmStr, world);
  cse498::Building blacksmith(2, blacksmithStr, world);

  CHECK(farm.GetName() == farmStr);
  CHECK(blacksmith.GetName() == blacksmithStr);

  farm.SetName("Farm2");
  blacksmith.SetName("Blacksmith2");
  CHECK(farm.GetName() == "Farm2");
  CHECK(blacksmith.GetName() == "Blacksmith2");
}

TEST_CASE("Test Building GetAllUpgrades",
          "[core][InteractiveWorld][Building]") {
  cse498::Building farm(1, "Farm", world);

  farm.AddUpgrade(cse498::ItemType::Wood, 25);
  farm.AddUpgrade(cse498::ItemType::Stone, 35);
  farm.AddUpgrade(cse498::ItemType::Metal, 50);

  const auto &upgrades = farm.GetAllUpgrades();

  REQUIRE(upgrades.size() == 3);
  CHECK(upgrades[0].item == cse498::ItemType::Wood);
  CHECK(upgrades[0].quantity == 25);
  CHECK(upgrades[1].item == cse498::ItemType::Stone);
  CHECK(upgrades[1].quantity == 35);
  CHECK(upgrades[2].item == cse498::ItemType::Metal);
  CHECK(upgrades[2].quantity == 50);
}

TEST_CASE("Test Building One Upgrade", "[core][InteractiveWorld][Building]") {
  std::string farmStr = "Farm";
  cse498::Building farm(1, farmStr, world);

  CHECK(farm.GetCurrentLevel() == 0);
  CHECK(farm.GetMaxLevel() == 0);
  CHECK(farm.GetNextUpgradeInfo() == std::nullopt);

  farm.AddUpgrade(cse498::ItemType::Wood, 25);

  CHECK(farm.GetCurrentLevel() == 0);
  CHECK(farm.GetMaxLevel() == 1);
  CHECK(farm.IsMaxLevel() == false);
  CHECK(farm.GetNextUpgradeLevel() == 1);
  const auto next_upgrade = farm.GetNextUpgradeInfo();
  REQUIRE(next_upgrade.has_value());
  CHECK(next_upgrade->quantity == 25);
  CHECK(next_upgrade->item == cse498::ItemType::Wood);
  // We need 25 wood, testing with too few items
  int currentWood = 5;

  // Check for not enough items error
  auto status = farm.Upgrade(cse498::ItemType::Wood, currentWood);
  CHECK(status.error() ==
        cse498::Building::UpgradeRejectionType::NotEnoughItems);
  CHECK(farm.GetCurrentLevel() == 0);
  // Check for incorrect item type
  auto status2 = farm.Upgrade(cse498::ItemType::Stone, currentWood);
  CHECK(status2.error() ==
        cse498::Building::UpgradeRejectionType::IncorrectItemType);
  CHECK(farm.GetCurrentLevel() == 0);
  // Check for no error, successful upgrade
  currentWood = 100;
  auto status3 = farm.Upgrade(cse498::ItemType::Wood, currentWood);
  CHECK(status3.has_value() == true);

  CHECK(farm.GetCurrentLevel() == 1);
  CHECK(farm.IsMaxLevel() == true);
  CHECK(farm.GetNextUpgradeInfo() == std::nullopt);

  // Check for upgrading already max level building
  auto status4 = farm.Upgrade(cse498::ItemType::Wood, currentWood);
  CHECK(status4.error() ==
        cse498::Building::UpgradeRejectionType::AlreadyMaxLevel);
  CHECK(farm.GetCurrentLevel() == 1);
  CHECK(farm.IsMaxLevel() == true);
}

TEST_CASE("Test Building Multiple Upgrades",
          "[core][InteractiveWorld][Building]") {
  std::string farmStr = "Farm";
  cse498::Building farm(1, farmStr, world);

  CHECK(farm.GetCurrentLevel() == 0);
  CHECK(farm.GetMaxLevel() == 0);

  farm.AddUpgrade(cse498::ItemType::Wood, 25);
  farm.AddUpgrade(cse498::ItemType::Stone, 35);
  farm.AddUpgrade(cse498::ItemType::Metal, 50);

  CHECK(farm.GetCurrentLevel() == 0);
  CHECK(farm.GetMaxLevel() == 3);

  int resources = 100;

  // Upgrade 0 -> 1
  auto status1 = farm.Upgrade(cse498::ItemType::Wood, resources);
  CHECK(status1.has_value() == true);
  CHECK(farm.GetCurrentLevel() == 1);
  CHECK(farm.IsMaxLevel() == false);
  // Upgrade 1 -> 2
  auto status2 = farm.Upgrade(cse498::ItemType::Stone, resources);
  CHECK(status2.has_value() == true);
  CHECK(farm.GetCurrentLevel() == 2);
  CHECK(farm.IsMaxLevel() == false);
  // Upgrade 2 -> 3 (Max in this case)
  auto status3 = farm.Upgrade(cse498::ItemType::Metal, resources);
  CHECK(status3.has_value() == true);
  CHECK(farm.GetCurrentLevel() == 3);
  CHECK(farm.IsMaxLevel() == true);
  // Attempting to level beyond max level
  auto status4 = farm.Upgrade(cse498::ItemType::Metal, resources);
  CHECK(status4.has_value() == false);
  CHECK(status4.error() ==
        cse498::Building::UpgradeRejectionType::AlreadyMaxLevel);
  CHECK(farm.GetCurrentLevel() == 3);
  CHECK(farm.IsMaxLevel() == true);
  // Attempting to level beyond max level
  auto status5 = farm.Upgrade(cse498::ItemType::Metal, resources);
  CHECK(status5.has_value() == false);
  CHECK(status5.error() ==
        cse498::Building::UpgradeRejectionType::AlreadyMaxLevel);
  CHECK(farm.GetCurrentLevel() == 3);
  CHECK(farm.IsMaxLevel() == true);
  CHECK(farm.GetNextUpgradeInfo() == std::nullopt);
}
