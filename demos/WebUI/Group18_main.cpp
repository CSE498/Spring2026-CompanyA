/// Build command from root: 
///   cmake -S . -B build
///   cmake --build build --target group18_demo
///
/// Run this command from root to open the demo:
///   emrun demos/WebUI/index.html --browser chrome --serve-root .
/// Images use absolute paths from the project root so you must use --serve-root .
/// Alternatively, run:
///   emrun demos/WebUI/index.html --no_browser --serve-root .
/// The open in browser:
///   http://localhost:6931/demos/WebUI/index.html

#ifdef __EMSCRIPTEN__

#include <algorithm>
#include <emscripten/emscripten.h>
#include "../../source/Interfaces/WebUI/interface/MockWorld.hpp"
#include "../../source/Interfaces/WebUI/interface/WebInterface.hpp"
#include "../../source/Agents/PacingAgent.hpp"

namespace {

using namespace cse498;

/// @class RunningMockWorld
/// @brief Extends MockWorld for the running application with setup and ticking.
class RunningMockWorld : public cse498::MockWorld {
public:
  /// @brief Updates the world state with the given time delta.
  /// @param millis The time delta in milliseconds.
  void Tick(double millis) {
    IncrementActionTimer(millis);
  }

  /// @brief Sets up the world with agents and interface.
  void Setup() {
    AddAgent<PacingAgent>("Pacer 1").SetLocation(WorldPosition{3,1});
    AddAgent<PacingAgent>("Pacer 2").SetLocation(WorldPosition{6,1});
    AddAgent<PacingAgent>("Guard 1").SetHorizontal().SetLocation(WorldPosition{7,7});
    AddAgent<PacingAgent>("Guard 2").SetHorizontal().ToggleDirection().SetLocation(WorldPosition{8,8});
    AddInterface<WebInterface>("Web UI").SetSymbol('@').SetLocation(WorldPosition{1,1});
  }
};

/// @struct App
/// @brief Main application structure managing the game loop.
struct App {
  /// @brief The running mock world instance.
  RunningMockWorld world;

  /// @brief Timestamp of the last frame in milliseconds.
  double last_time_ms{0.0};

  /// @brief Constructs the application and sets up the world.
  App() {
    // Add WebInterface to the world and initialize it.
    world.Setup();
  }

  /// @brief Main loop callback for Emscripten animation frame.
  /// @param currentTimeMs Current time in milliseconds.
  /// @param userData Pointer to the App instance.
  /// @return EM_BOOL indicating whether to continue the loop.
  static EM_BOOL MainLoop(double currentTimeMs, void* userData) {
    auto* app = static_cast<App*>(userData);

    // Handle the very first frame
    if (app->last_time_ms == 0.0) {
      app->last_time_ms = currentTimeMs;
    }

    const double delta = std::max(0.0, currentTimeMs - app->last_time_ms);
    app->last_time_ms = currentTimeMs;

    app->world.Tick(delta);
    app->world.Run();

    return EM_TRUE;
  }
};

} // anonymous namespace

/// @brief Main entry point for the application.
/// @return Exit code.
int main() {
  static App app;

  emscripten_request_animation_frame_loop(&App::MainLoop, &app);

  return 0;
}

#endif
