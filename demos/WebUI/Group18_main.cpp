/// Build command from root:
///   cmake -S . -B build
///   cmake --build build --target group18_demo
///
/// Run this command from root to open the demo:
///   emrun demos/WebUI/index.html --browser chrome --serve-root .
/// Images use absolute paths from the project root so you must use --serve-root .
/// Alternatively, run:
///   emrun demos/WebUI/index.html --no_browser --serve-root .
/// Then open in browser:
///   http://localhost:6931/demos/WebUI/index.html
///
/// This file was modified with AI assistance for WebUI integration
/// with InteractiveWorld and DungeonWorld systems.

#ifdef __EMSCRIPTEN__

#include <algorithm>
#include <emscripten/emscripten.h>
#include <memory>
#include "../../source/Agents/AI/FetchAgent.hpp"
#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/Interfaces/WebUI/interface/WebInterface.hpp"
#include "../../source/Worlds/Dungeon/DungeonWorld.hpp"
#include "../../source/Worlds/Hub/InteractiveWorld.hpp"

namespace {

using namespace cse498;

std::unique_ptr<InteractiveWorld> SetupOverworld() {
    auto overworld = std::make_unique<InteractiveWorld>();

    overworld->GetInventory().AddItem(ItemType::Wood, 10);
    overworld->GetInventory().AddItem(ItemType::Stone, 5);

        auto townHallPtr = std::make_unique<TownHall>(overworld->GetNextAgentId(), "Town Hall", *overworld,
                                                      overworld->GetInventoryPtr());
        townHallPtr->SetSymbol('T');
        TownHall& townHall = overworld->AddAgent(std::move(townHallPtr));
        overworld->AddTownHall(townHall, WorldPosition{11, 5});

        // Buildings now act as both upgrade points and intermediate storage.
        Building& lumberYard = overworld->AddAgent<Building>("Lumber Yard");
        lumberYard.SetSymbol('W');
        lumberYard.AddUpgrade(ItemType::Wood, 15);

    Building& quarry = overworld->AddAgent<Building>("Quarry");
    quarry.SetSymbol('Q');
    quarry.AddUpgrade(ItemType::Wood, 50);
    quarry.AddUpgrade(ItemType::Stone, 50);
    quarry.AddUpgrade(ItemType::Metal, 35);

    Building& mine = overworld->AddAgent<Building>("Mine");
    mine.SetSymbol('M');
    mine.AddUpgrade(ItemType::Stone, 100);
    mine.AddUpgrade(ItemType::Metal, 50);
    mine.AddUpgrade(ItemType::Metal, 100);

        // Resource spawns occupy the three corners farthest from the player start.
        auto woodSpawnPtr =
                std::make_unique<ResourceSpawn>(overworld->GetNextAgentId(), "Wood Spawn", *overworld, ItemType::Wood);
        woodSpawnPtr->SetSymbol('l');
        ResourceSpawn& woodSpawn = overworld->AddAgent(std::move(woodSpawnPtr));
        overworld->AddResourceSpawn(woodSpawn, WorldPosition{21, 1});

        auto stoneSpawnPtr = std::make_unique<ResourceSpawn>(overworld->GetNextAgentId(), "Stone Spawn", *overworld,
                                                             ItemType::Stone);
        stoneSpawnPtr->SetSymbol('q');
        ResourceSpawn& stoneSpawn = overworld->AddAgent(std::move(stoneSpawnPtr));
        overworld->AddResourceSpawn(stoneSpawn, WorldPosition{1, 9});

        auto metalSpawnPtr = std::make_unique<ResourceSpawn>(overworld->GetNextAgentId(), "Metal Spawn", *overworld,
                                                             ItemType::Metal);
        metalSpawnPtr->SetSymbol('m');
        ResourceSpawn& metalSpawn = overworld->AddAgent(std::move(metalSpawnPtr));
        overworld->AddResourceSpawn(metalSpawn, WorldPosition{21, 9});


        // Resource Producers
        std::shared_ptr<ResourceProducer> woodProducer =
                std::make_shared<ResourceProducer>(lumberYard, woodSpawn, ItemType::Wood, 2);

        std::shared_ptr<ResourceProducer> stoneProducer =
                std::make_shared<ResourceProducer>(quarry, stoneSpawn, ItemType::Stone, 1);

        std::shared_ptr<ResourceProducer> metalProducer =
                std::make_shared<ResourceProducer>(mine, metalSpawn, ItemType::Metal, 0.5);

    overworld->AddProducer(woodProducer);
    overworld->AddProducer(stoneProducer);
    overworld->AddProducer(metalProducer);

        overworld->AddBuilding(lumberYard, WorldPosition{15, 3});
        overworld->AddBuilding(quarry, WorldPosition{5, 7});
        overworld->AddBuilding(mine, WorldPosition{15, 7});

        lumberYard.SetSymbol('W');
        quarry.SetSymbol('Q');
        mine.SetSymbol('M');

        auto configureFetcher = [](FetchAgent& fetcher, AgentBase& origin, AgentBase& deposit, ItemType itemType,
                                   char symbol, WorldPosition position) {
            fetcher.SetOrigin(origin).SetDepositPoint(deposit).SetItemType(itemType).SetSymbol(symbol).SetLocation(
                    position);
        };

        // Two-stage hauling pipeline for each resource:
        // ResourceSpawn -> producer building -> TownHall.
        FetchAgent& woodToLumberYard = overworld->AddAgent<FetchAgent>("Wood To Lumber Yard");
        configureFetcher(woodToLumberYard, woodSpawn, lumberYard, ItemType::Wood, '1', WorldPosition{20, 2});

        FetchAgent& woodToTownHall = overworld->AddAgent<FetchAgent>("Lumber Yard To Town Hall");
        configureFetcher(woodToTownHall, lumberYard, townHall, ItemType::Wood, '2', WorldPosition{15, 4});

        FetchAgent& stoneToQuarry = overworld->AddAgent<FetchAgent>("Stone To Quarry");
        configureFetcher(stoneToQuarry, stoneSpawn, quarry, ItemType::Stone, '3', WorldPosition{2, 8});

        FetchAgent& stoneToTownHall = overworld->AddAgent<FetchAgent>("Quarry To Town Hall");
        configureFetcher(stoneToTownHall, quarry, townHall, ItemType::Stone, '4', WorldPosition{7, 6});

        FetchAgent& metalToMine = overworld->AddAgent<FetchAgent>("Metal To Mine");
        configureFetcher(metalToMine, metalSpawn, mine, ItemType::Metal, '5', WorldPosition{20, 8});

        FetchAgent& metalToTownHall = overworld->AddAgent<FetchAgent>("Mine To Town Hall");
        configureFetcher(metalToTownHall, mine, townHall, ItemType::Metal, '6', WorldPosition{15, 6});

        ResourceManagementAgent& resourceManager = overworld->AddAgent<ResourceManagementAgent>("Resource Manager");
        resourceManager.SetInventory(overworld->GetInventoryPtr()).SetSymbol('7');
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

        webInterface = std::make_unique<WebInterface>(std::move(overworld), std::move(dungeon));
    }

    static void MainLoop() {
        if (webInterface->GetCurrentState() == WebInterface::WebState::QUIT) {
            emscripten_cancel_main_loop();
            return;
        }
        webInterface->RunFrame(emscripten_performance_now());
    }
};

int main() {
    App app{};

    emscripten_set_main_loop(&App::MainLoop, 0, true);

    return 0;
}

#endif
