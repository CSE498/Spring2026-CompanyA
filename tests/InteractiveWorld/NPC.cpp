#include "catch2/catch.hpp"

#include "../../source/Worlds/Hub/NPC.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

/// used codex for some testcase generation

TEST_CASE("NPC displays building upgrade information", "[core][InteractiveWorld][NPC]") {
    std::string farmStr = "Farm";
    auto building = std::make_shared<cse498::Building>(farmStr);

    building->AddUpgrade(cse498::ItemType::Wood, 25);

    cse498::NPC npc(building);

    std::ostringstream output;
    CHECK(npc.GetUpgradeUI() == "Farm - level: 0 | next upgrade: 25 Wood for level 1");

    npc.ShowUpgradeUI(output);

    CHECK(output.str() == "Farm - level: 0 | next upgrade: 25 Wood for level 1\n");
}

TEST_CASE("NPC upgrades a building and spends inventory resources", "[core][InteractiveWorld][NPC]") {
    std::string blacksmithStr = "Blacksmith";
    auto building = std::make_shared<cse498::Building>(blacksmithStr);

    building->AddUpgrade(cse498::ItemType::Stone, 10);

    cse498::NPC npc(building);
    cse498::InteractiveWorldInventory inventory;

    inventory.AddItem(cse498::ItemType::Stone, 15);

    auto status = npc.AttemptUpgrade(inventory);

    CHECK(status.has_value() == true);
    CHECK(building->GetCurrentLevel() == 1);
    CHECK(inventory.GetAmount(cse498::ItemType::Stone) == 5);
}

TEST_CASE("NPC rejects upgrades when inventory is missing resources", "[core][InteractiveWorld][NPC]") {
    std::string workshopStr = "Workshop";
    auto building = std::make_shared<cse498::Building>(workshopStr);

    building->AddUpgrade(cse498::ItemType::Metal, 20);

    cse498::NPC npc(building);
    cse498::InteractiveWorldInventory inventory;

    inventory.AddItem(cse498::ItemType::Metal, 4);

    auto status = npc.AttemptUpgrade(inventory);

    CHECK(status.has_value() == false);
    CHECK(status.error() == cse498::Building::UpgradeRejectionType::NotEnoughItems);
    CHECK(building->GetCurrentLevel() == 0);
    CHECK(inventory.GetAmount(cse498::ItemType::Metal) == 4);
}

TEST_CASE("NPC position and symbol accessors update state", "[core][InteractiveWorld][NPC]") {
    auto building = std::make_shared<cse498::Building>("Farm");
    cse498::NPC npc(building);

    npc.SetPosition(cse498::WorldPosition{4, 7});
    npc.SetSymbol('F');

    CHECK(npc.GetPosition() == cse498::WorldPosition{4, 7});
    CHECK(npc.GetSymbol() == 'F');
}

TEST_CASE("NPC interact prints upgrade information", "[core][InteractiveWorld][NPC]") {
    auto building = std::make_shared<cse498::Building>("Farm");
    building->AddUpgrade(cse498::ItemType::Wood, 25);

    cse498::NPC npc(building);

    std::ostringstream output;
    std::streambuf* originalBuffer = std::cout.rdbuf(output.rdbuf());

    npc.Interact();
    std::cout.rdbuf(originalBuffer);

    CHECK(output.str() == "Farm - level: 0 | next upgrade: 25 Wood for level 1\n");
}
