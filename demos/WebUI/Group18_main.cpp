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
class App {
public:
    inline static std::unique_ptr<cse498::WebInterface> webInterface = nullptr;
    App() {
        webInterface =
                std::make_unique<WebInterface>(std::make_unique<InteractiveWorld>(), std::make_unique<DungeonWorld>());
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
