#include "../../../source/core/InteractiveWorld/ResourceProducer.hpp"
#include "../../../source/core/InteractiveWorld/InteractiveWorldInventory.hpp"
#include "catch2/catch.hpp"
#include <iostream>

#include <string>

bool ApproxEqual(float a, float b, float tolerance = 1e-5f) {
  return std::abs(a - b) <= tolerance;
}

TEST_CASE("Test ResourceProducer Constructor",
          "[core][InteractiveWorld][ResourceProducer]") {
  std::string farmStr = "Farm";
  std::shared_ptr<cse498::Building> farm =
      std::make_shared<cse498::Building>("Farm");
  CHECK(farm->GetName() == farmStr);

  cse498::InteractiveWorldInventory inv;
  float baseRate = 1.0f;
  cse498::ResourceProducer producer(farm, inv, ItemType::Wood, baseRate);

  CHECK(producer.GetRate() == baseRate);
}

TEST_CASE("Test ResourceProducer Rate increase",
          "[core][InteractiveWorld][ResourceProducer]") {
  std::string farmStr = "Farm";
  std::shared_ptr<cse498::Building> farm =
      std::make_shared<cse498::Building>("Farm");
  CHECK(farm->GetName() == farmStr);
  farm->AddUpgrade(ItemType::Wood, 5);
  farm->AddUpgrade(ItemType::Wood, 5);
  farm->AddUpgrade(ItemType::Wood, 5);
  farm->SetRateModifier(0.25f);

  cse498::InteractiveWorldInventory inv;
  inv.AddItem(ItemType::Wood, 15);
  float baseRate = 1.0f;
  cse498::ResourceProducer producer(farm, inv, ItemType::Wood, baseRate);

  CHECK(producer.GetRate() == baseRate);
  ItemType type = ItemType::Wood;

  int inventoryWoodCount = inv.GetAmount(type);
  farm->Upgrade(type, inventoryWoodCount);
  CHECK(farm->GetCurrentLevel() == 1);
  producer.CalculateRate();
  CHECK(ApproxEqual(producer.GetRate(), 1.25f));

  farm->Upgrade(type, inventoryWoodCount);
  CHECK(farm->GetCurrentLevel() == 2);
  producer.CalculateRate();
  CHECK(ApproxEqual(producer.GetRate(), 1.5f));

  farm->Upgrade(type, inventoryWoodCount);
  CHECK(farm->GetCurrentLevel() == 3);
  producer.CalculateRate();
  CHECK(ApproxEqual(producer.GetRate(), 1.75f));
}