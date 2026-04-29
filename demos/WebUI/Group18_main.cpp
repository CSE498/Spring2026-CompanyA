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
#include "../../source/Worlds/DemoG2/DemoSimpleWorldG2.hpp"

namespace {

using namespace cse498;
class App {
public:
    inline static std::unique_ptr<cse498::WebInterface> webInterface = nullptr;
    App() {
        auto overworld = std::make_unique<DemoSimpleWorldG2>();
        auto dungeon = std::make_unique<DungeonWorld>();

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




        lumberYard.SetSymbol('W');
        quarry.SetSymbol('Q');
        mine.SetSymbol('M');

        auto configureFetcher = [](FetchAgent& fetcher, AgentBase& origin, AgentBase& deposit, ItemType itemType,
                                   char symbol, WorldPosition position) {
            fetcher.SetOrigin(origin).SetDepositPoint(deposit).SetItemType(itemType).SetSymbol(symbol).SetLocation(
                    position);
        };


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

} // namespace

int main() {
    App app{};

    emscripten_set_main_loop(&App::MainLoop, 0, true);

    return 0;
}

#endif
