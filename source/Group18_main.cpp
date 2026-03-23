#ifdef __EMSCRIPTEN__

#include <emscripten/emscripten.h>
#include "Interfaces/WebUI/interface/MockWorld.hpp"
#include "Interfaces/WebUI/interface/WebInterface.hpp"
#include "./Agents/PacingAgent.hpp"

namespace {

using namespace cse498;

class RunningMockWorld : public cse498::MockWorld {
public:
  void Tick(double millis) {
    IncrementActionTimer(millis);
  }

  void Setup() {
    AddAgent<PacingAgent>("Pacer 1").SetLocation(WorldPosition{3,1});
    AddAgent<PacingAgent>("Pacer 2").SetLocation(WorldPosition{6,1});
    AddAgent<PacingAgent>("Guard 1").SetHorizontal().SetLocation(WorldPosition{7,7});
    AddAgent<PacingAgent>("Guard 2").SetHorizontal().ToggleDirection().SetLocation(WorldPosition{8,8});
    AddInterface<WebInterface>("Web UI").SetSymbol('@').SetLocation(WorldPosition{1,1});
  }
};

struct App {
  RunningMockWorld world;
  double last_time_ms{0.0};

  App() {
    // Add WebInterface to the world and initialize it.
    world.Setup();
  }

  static EM_BOOL MainLoop(double currentTimeMs, void* userData) {
  auto* app = static_cast<App*>(userData);
  if (!app) return EM_FALSE;

  // Handle the very first frame
  if (app->last_time_ms == 0.0) {
    app->last_time_ms = currentTimeMs;
  }

  const double delta = currentTimeMs - app->last_time_ms;
  app->last_time_ms = currentTimeMs;

  if (delta >= 0) {
    app->world.Tick(delta);
    app->world.Run();
  }

  return EM_TRUE; 
}
};

} // anonymous namespace

int main() {
  static App * app = new App();

  emscripten_request_animation_frame_loop(&App::MainLoop, app);

  return 0;
}

#endif