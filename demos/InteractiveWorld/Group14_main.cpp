/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "../../source/Agents/AI/FetchAgent.hpp"
#include "../../source/Agents/Classic/FarmingAgent.hpp"
#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/Interfaces/TrashInterface.hpp"
#include "../../source/Worlds/Hub/Building.hpp"
#include "../../source/Worlds/Hub/InteractiveWorld.hpp"
#include "../../source/Worlds/Hub/InteractiveWorldInventory.hpp"
#include "../../source/Worlds/Hub/InteractiveWorldSaveManager.hpp"
#include "../../source/Worlds/Hub/ResourceProducer.hpp"
#include "../../source/Worlds/Hub/ResourceSpawn.hpp"
#include "../../source/Worlds/Hub/TownHall.hpp"
#include "../../source/Worlds/MazeWorld.hpp"

#include <iostream>
#include <memory>
#include <string>

using namespace cse498;

int main() {
    std::shared_ptr<InteractiveWorld> world = std::make_shared<InteractiveWorld>();

    // InteractiveWorldSaveManager saveManager;

    world->GetInventory().AddItem(ItemType::Wood, 10);
    world->GetInventory().AddItem(ItemType::Stone, 5);

    world->AddAgent<TrashInterface>("Interface").SetSymbol('@').SetLocation(WorldPosition{1, 1});

    auto townHallPtr =
            std::make_unique<TownHall>(world->GetNextAgentId(), "Town Hall", *world, world->GetInventoryPtr());
    townHallPtr->SetSymbol('T');
    TownHall& townHall = world->AddAgent(std::move(townHallPtr));
    world->AddTownHall(townHall, WorldPosition{11, 5});

    // Buildings
    Building& lumberYard = world->AddAgent<Building>("Lumber Yard");
    lumberYard.SetSymbol('L');
    lumberYard.AddUpgrade(ItemType::Wood, 15);

    Building& quarry = world->AddAgent<Building>("Quarry");
    quarry.SetSymbol('Q');
    quarry.AddUpgrade(ItemType::Wood, 50);
    quarry.AddUpgrade(ItemType::Stone, 50);
    quarry.AddUpgrade(ItemType::Metal, 35);

    Building& mine = world->AddAgent<Building>("Mine");
    mine.SetSymbol('M');
    mine.AddUpgrade(ItemType::Stone, 100);
    mine.AddUpgrade(ItemType::Metal, 50);
    mine.AddUpgrade(ItemType::Metal, 100);

    // Resource banks sit between the corner spawns and the center town hall.
    ResourceBank& woodBank = world->AddAgent<ResourceBank>("Wood Bank");
    woodBank.SetSymbol('B');
    world->AddResourceBank(woodBank, WorldPosition{16, 3});

    ResourceBank& stoneBank = world->AddAgent<ResourceBank>("Stone Bank");
    stoneBank.SetSymbol('B');
    world->AddResourceBank(stoneBank, WorldPosition{6, 7});

    ResourceBank& metalBank = world->AddAgent<ResourceBank>("Metal Bank");
    metalBank.SetSymbol('B');
    world->AddResourceBank(metalBank, WorldPosition{16, 7});

    // Resource spawns occupy the three corners farthest from the player start.
    auto woodSpawnPtr = std::make_unique<ResourceSpawn>(world->GetNextAgentId(), "Wood Spawn", *world, ItemType::Wood);
    woodSpawnPtr->SetSymbol('l');
    ResourceSpawn& woodSpawn = world->AddAgent(std::move(woodSpawnPtr));
    world->AddResourceSpawn(woodSpawn, WorldPosition{21, 1});

    auto stoneSpawnPtr =
            std::make_unique<ResourceSpawn>(world->GetNextAgentId(), "Stone Spawn", *world, ItemType::Stone);
    stoneSpawnPtr->SetSymbol('q');
    ResourceSpawn& stoneSpawn = world->AddAgent(std::move(stoneSpawnPtr));
    world->AddResourceSpawn(stoneSpawn, WorldPosition{1, 9});

    auto metalSpawnPtr =
            std::make_unique<ResourceSpawn>(world->GetNextAgentId(), "Metal Spawn", *world, ItemType::Metal);
    metalSpawnPtr->SetSymbol('m');
    ResourceSpawn& metalSpawn = world->AddAgent(std::move(metalSpawnPtr));
    world->AddResourceSpawn(metalSpawn, WorldPosition{21, 9});


    // Resource Producers
    std::shared_ptr<ResourceProducer> woodProducer =
            std::make_shared<ResourceProducer>(lumberYard, woodSpawn, ItemType::Wood, 2);

    std::shared_ptr<ResourceProducer> stoneProducer =
            std::make_shared<ResourceProducer>(quarry, stoneSpawn, ItemType::Stone, 1);

    std::shared_ptr<ResourceProducer> metalProducer =
            std::make_shared<ResourceProducer>(mine, metalSpawn, ItemType::Metal, 0.5);

    world->AddProducer(woodProducer);
    world->AddProducer(stoneProducer);
    world->AddProducer(metalProducer);

    world->AddBuilding(lumberYard, WorldPosition{15, 3});
    world->AddBuilding(quarry, WorldPosition{5, 7});
    world->AddBuilding(mine, WorldPosition{15, 7});

    lumberYard.SetSymbol('L');
    quarry.SetSymbol('Q');
    mine.SetSymbol('M');

    auto configureFetcher = [](FetchAgent& fetcher, AgentBase& origin, AgentBase& deposit, ItemType itemType,
                               char symbol, WorldPosition position) {
        fetcher.SetOrigin(origin).SetDepositPoint(deposit).SetItemType(itemType).SetSymbol(symbol).SetLocation(
                position);
    };

    // Two-stage hauling pipeline for each resource:
    // ResourceSpawn -> ResourceBank -> TownHall.
    FetchAgent& woodToBank = world->AddAgent<FetchAgent>("Wood To Bank");
    configureFetcher(woodToBank, woodSpawn, woodBank, ItemType::Wood, '1', WorldPosition{20, 2});

    FetchAgent& woodToTownHall = world->AddAgent<FetchAgent>("Wood To Town Hall");
    configureFetcher(woodToTownHall, woodBank, townHall, ItemType::Wood, '2', WorldPosition{15, 4});

    FetchAgent& stoneToBank = world->AddAgent<FetchAgent>("Stone To Bank");
    configureFetcher(stoneToBank, stoneSpawn, stoneBank, ItemType::Stone, '3', WorldPosition{2, 8});

    FetchAgent& stoneToTownHall = world->AddAgent<FetchAgent>("Stone To Town Hall");
    configureFetcher(stoneToTownHall, stoneBank, townHall, ItemType::Stone, '4', WorldPosition{7, 6});

    FetchAgent& metalToBank = world->AddAgent<FetchAgent>("Metal To Bank");
    configureFetcher(metalToBank, metalSpawn, metalBank, ItemType::Metal, '5', WorldPosition{20, 8});

    FetchAgent& metalToTownHall = world->AddAgent<FetchAgent>("Metal To Town Hall");
    configureFetcher(metalToTownHall, metalBank, townHall, ItemType::Metal, '6', WorldPosition{15, 6});

    FarmingAgent& woodAgent = world->AddAgent<FarmingAgent>("Wood Farmer");
    woodAgent.SetAssignedBuilding(&lumberYard);
    woodAgent.SetHomePosition(WorldPosition{14, 2});
    woodAgent.SetSymbol('7');
    woodAgent.SetLocation(WorldPosition{14, 4});


    InteractiveWorldSaveManager saveManager;
    if (saveManager.Load(*world, "interactive_world_save.json")) {
        std::cout << "Loaded existing save file.\n";
    } else {
        std::cout << "No save file found. Starting new game.\n";
    }

    world->Run();
}
