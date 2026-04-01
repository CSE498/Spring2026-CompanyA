#include <memory>

#include "../tools/ActionLog.hpp"
#include "../tools/DataLog.hpp"
#include "AnalyticsManager.hpp"
#include "HeatMap.hpp"
#include "ReplayDriver.hpp"

// #include "../tools/ActionLog.hpp"
int main() {
  cse498::AnalyticsManager analytics;
  analytics.LogHealth(100.0);
  analytics.LogEnemiesTracked(5);
  analytics.LogDamageDealt(50.0);

  analytics.LogHealth(80.0);
  analytics.LogEnemiesTracked(3);
  analytics.LogDamageDealt(25.0);

  analytics.LogHealth(100.0);
  analytics.LogEnemiesTracked(1);
  analytics.LogDamageDealt(50.0);

  cse498::ActionLog log;
  log.LogAction(1, "move", cse498::WorldPosition{0, 0},
                cse498::WorldPosition{0, 1});
  log.LogAction(1, "move", cse498::WorldPosition{0, 1},
                cse498::WorldPosition{0, 2});
  log.LogAction(1, "move", cse498::WorldPosition{0, 2},
                cse498::WorldPosition{0, 3});
  log.LogAction(1, "move", cse498::WorldPosition{0, 3},
                cse498::WorldPosition{0, 4});
  log.LogAction(1, "move", cse498::WorldPosition{0, 4},
                cse498::WorldPosition{0, 5});
  log.LogAction(1, "move", cse498::WorldPosition{0, 5},
                cse498::WorldPosition{1, 5});
  log.LogAction(1, "move", cse498::WorldPosition{1, 5},
                cse498::WorldPosition{2, 5});
  log.LogAction(1, "move", cse498::WorldPosition{2, 5},
                cse498::WorldPosition{2, 4});
  log.LogAction(1, "move", cse498::WorldPosition{2, 4},
                cse498::WorldPosition{2, 3});
  log.LogAction(1, "move", cse498::WorldPosition{2, 3},
                cse498::WorldPosition{2, 3});

  cse498::ReplayDriver driver(std::make_shared<cse498::ActionLog>(log));
  driver.ReplayFullGame();

  cse498::HeatMap heatmap(log, std::pair<int, int>{5, 5},
                          std::pair<int, int>{10, 21});
  heatmap.OutPutHeatMap();

  return 0;
}