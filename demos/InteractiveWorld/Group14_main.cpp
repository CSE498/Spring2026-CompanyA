/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "../../source/Agents/AI/FetchAgent.hpp"
#include "../../source/Agents/Classic/ResourceManagementAgent.hpp"
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

    // Buildings now act as both upgrade points and intermediate storage.
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
    // ResourceSpawn -> producer building -> TownHall.
    FetchAgent& woodToLumberYard = world->AddAgent<FetchAgent>("Wood To Lumber Yard");
    configureFetcher(woodToLumberYard, woodSpawn, lumberYard, ItemType::Wood, '1', WorldPosition{20, 2});

    FetchAgent& woodToTownHall = world->AddAgent<FetchAgent>("Lumber Yard To Town Hall");
    configureFetcher(woodToTownHall, lumberYard, townHall, ItemType::Wood, '2', WorldPosition{15, 4});

    FetchAgent& stoneToQuarry = world->AddAgent<FetchAgent>("Stone To Quarry");
    configureFetcher(stoneToQuarry, stoneSpawn, quarry, ItemType::Stone, '3', WorldPosition{2, 8});

    FetchAgent& stoneToTownHall = world->AddAgent<FetchAgent>("Quarry To Town Hall");
    configureFetcher(stoneToTownHall, quarry, townHall, ItemType::Stone, '4', WorldPosition{7, 6});

    FetchAgent& metalToMine = world->AddAgent<FetchAgent>("Metal To Mine");
    configureFetcher(metalToMine, metalSpawn, mine, ItemType::Metal, '5', WorldPosition{20, 8});

    FetchAgent& metalToTownHall = world->AddAgent<FetchAgent>("Mine To Town Hall");
    configureFetcher(metalToTownHall, mine, townHall, ItemType::Metal, '6', WorldPosition{15, 6});

    ResourceManagementAgent& resourceManager = world->AddAgent<ResourceManagementAgent>("Resource Manager");
    resourceManager.SetInventory(world->GetInventoryPtr()).SetSymbol('7');
    resourceManager.AddManagedBuilding(lumberYard, true);
    resourceManager.AddManagedBuilding(quarry, false);
    resourceManager.AddManagedBuilding(mine, false);
    resourceManager.SetLocation(WorldPosition{2, 3});

    woodToLumberYard.SetActive(true);
    woodToTownHall.SetActive(true);

    stoneToQuarry.SetActive(false);
    stoneToTownHall.SetActive(false);

    metalToMine.SetActive(false);
    metalToTownHall.SetActive(false);

    resourceManager.AddHireableLane("Quarry Lane", stoneToQuarry, stoneToTownHall, quarry, 10);
    resourceManager.AddHireableLane("Mine Lane", metalToMine, metalToTownHall, mine, 20);


    InteractiveWorldSaveManager saveManager;
    if (saveManager.Load(*world, "interactive_world_save.json")) {
        std::cout << "Loaded existing save file.\n";
    } else {
        std::cout << "No save file found. Starting new game.\n";
    }

    world->Run();
}
