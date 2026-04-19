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
#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/Interfaces/WebUI/interface/WebInterface.hpp"
#include "../../source/Worlds/Dungeon/DungeonWorld.hpp"
#include "../../source/Worlds/Hub/InteractiveWorld.hpp"

namespace {

using namespace cse498;

} // anonymous namespace

class App {
public:
    inline static std::unique_ptr<cse498::WebInterface> webInterface = nullptr;
    App() {
        auto overworld = std::make_unique<InteractiveWorld>();
        auto dungeon = std::make_unique<DungeonWorld>();

        overworld->GetInventory().AddItem(ItemType::Wood, 10);
        overworld->GetInventory().AddItem(ItemType::Stone, 5);

        overworld->AddAgent<PacingAgent>("Pacer 1").SetLocation(WorldPosition{3, 1});
        overworld->AddAgent<PacingAgent>("Pacer 2").SetLocation(WorldPosition{6, 1});
        overworld->AddAgent<PacingAgent>("Guard 1").SetHorizontal().SetLocation(WorldPosition{7, 7});
        overworld->AddAgent<PacingAgent>("Guard 2").SetHorizontal().ToggleDirection().SetLocation(WorldPosition{8, 8});

        // Buildings
        Building& lumberYard = overworld->AddAgent<Building>("Lumber Yard");
        lumberYard.SetSymbol('L');
        lumberYard.AddUpgrade(ItemType::Wood, 15);
        lumberYard.SetLocation(WorldPosition{2, 1});

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

        // Resource spawns
        auto woodSpawnPtr =
                std::make_unique<ResourceSpawn>(overworld->GetNextAgentId(), "Wood Spawn", *overworld, ItemType::Wood);
        woodSpawnPtr->SetSymbol('l');
        ResourceSpawn& woodSpawn = overworld->AddAgent(std::move(woodSpawnPtr));
        woodSpawn.SetLocation(WorldPosition{5, 8});

        auto stoneSpawnPtr = std::make_unique<ResourceSpawn>(overworld->GetNextAgentId(), "Stone Spawn", *overworld,
                                                             ItemType::Stone);
        stoneSpawnPtr->SetSymbol('q');
        ResourceSpawn& stoneSpawn = overworld->AddAgent(std::move(stoneSpawnPtr));
        stoneSpawn.SetLocation(WorldPosition{9, 9});

        auto metalSpawnPtr = std::make_unique<ResourceSpawn>(overworld->GetNextAgentId(), "Metal Spawn", *overworld,
                                                             ItemType::Metal);
        metalSpawnPtr->SetSymbol('m');
        ResourceSpawn& metalSpawn = overworld->AddAgent(std::move(metalSpawnPtr));
        metalSpawn.SetLocation(WorldPosition{13, 5});


        // Resource Producers
        std::shared_ptr<ResourceProducer> woodProducer =
                std::make_shared<ResourceProducer>(lumberYard, overworld->GetInventory(), ItemType::Wood, 2);

        std::shared_ptr<ResourceProducer> stoneProducer =
                std::make_shared<ResourceProducer>(quarry, overworld->GetInventory(), ItemType::Stone, 1);

        std::shared_ptr<ResourceProducer> metalProducer =
                std::make_shared<ResourceProducer>(mine, overworld->GetInventory(), ItemType::Metal, 0.5);

        overworld->AddProducer(woodProducer);
        overworld->AddProducer(stoneProducer);
        overworld->AddProducer(metalProducer);

        overworld->AddBuilding(lumberYard, WorldPosition{5, 5});
        overworld->AddBuilding(quarry, WorldPosition{11, 7});
        overworld->AddBuilding(mine, WorldPosition{16, 9});

        lumberYard.SetSymbol('L');
        quarry.SetSymbol('Q');
        mine.SetSymbol('M');

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
