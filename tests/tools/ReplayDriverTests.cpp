// ReplayDriverTests.cpp

#include <memory>

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/ActionLog.hpp"
#include "../../source/DataAnalytics/ReplayDriver.hpp"
#include "../../source/core/WorldPosition.hpp"

namespace {

std::shared_ptr<cse498::ActionLog> MakeFullGameLog() {
  auto log = std::make_shared<cse498::ActionLog>();

  log->LogAction(1, "move", cse498::WorldPosition{0, 0},
                 cse498::WorldPosition{0, 1});
  log->LogAction(1, "move", cse498::WorldPosition{0, 1},
                 cse498::WorldPosition{0, 2});
  log->LogAction(1, "move", cse498::WorldPosition{0, 2},
                 cse498::WorldPosition{0, 3});
  log->LogAction(1, "move", cse498::WorldPosition{0, 3},
                 cse498::WorldPosition{0, 4});
  log->LogAction(1, "move", cse498::WorldPosition{0, 4},
                 cse498::WorldPosition{0, 5});
  log->LogAction(1, "move", cse498::WorldPosition{0, 5},
                 cse498::WorldPosition{1, 5});
  log->LogAction(1, "move", cse498::WorldPosition{1, 5},
                 cse498::WorldPosition{2, 5});
  log->LogAction(1, "move", cse498::WorldPosition{2, 5},
                 cse498::WorldPosition{2, 4});
  log->LogAction(1, "move", cse498::WorldPosition{2, 4},
                 cse498::WorldPosition{2, 3});
  log->LogAction(1, "move", cse498::WorldPosition{2, 3},
                 cse498::WorldPosition{2, 3});
  return log;
}

std::shared_ptr<cse498::ActionLog> MakeTimedLog() {
  auto log = std::make_shared<cse498::ActionLog>();

  log->UpdateTime(1.0);
  log->LogAction(1, "move", cse498::WorldPosition{0, 0},
                 cse498::WorldPosition{0, 1});

  log->UpdateTime(2.0);
  log->LogAction(1, "move", cse498::WorldPosition{0, 1},
                 cse498::WorldPosition{0, 2});

  log->UpdateTime(3.0);
  log->LogAction(1, "move", cse498::WorldPosition{0, 2},
                 cse498::WorldPosition{0, 3});

  return log;
}

}

TEST_CASE("ReplayDriver ReplayFullGame runs through valid actions",
          "[ReplayDriver]") {
  auto log = MakeFullGameLog();

  cse498::ReplayDriver driver(log, 0);

  REQUIRE_NOTHROW(driver.ReplayFullGame());
}

TEST_CASE("ReplayDriver ReplayByTimeRange replays only selected time window",
          "[ReplayDriver]") {
  auto log = MakeTimedLog();

  cse498::ReplayDriver driver(log, 0);

  REQUIRE_NOTHROW(driver.ReplayByTimeRange(1.5, 2.5));
}

TEST_CASE("ReplayDriver ReplayByTimeRange handles empty ranges",
          "[ReplayDriver]") {
  auto log = MakeTimedLog();

  cse498::ReplayDriver driver(log, 0);

  REQUIRE_NOTHROW(driver.ReplayByTimeRange(10.0, 20.0));
}

TEST_CASE("ReplayDriver ReplayFullGame handles empty logs",
          "[ReplayDriver]") {
  auto log = std::make_shared<cse498::ActionLog>();

  cse498::ReplayDriver driver(log, 0);

  REQUIRE_NOTHROW(driver.ReplayFullGame());
}