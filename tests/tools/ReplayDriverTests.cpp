// ReplayDriverTests.cpp

#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../../source/Analyze/ReplayDriver.hpp"
#include "../../source/core/WorldPosition.hpp"
#include "../../source/tools/ActionLog.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

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

std::string CaptureStdout(const std::function<void()> &fn) {
  std::ostringstream captured;
  std::streambuf *original_buf = std::cout.rdbuf(captured.rdbuf());

  fn();

  std::cout.rdbuf(original_buf);
  return captured.str();
}

std::string ExpectedStartingBoard() {
  std::vector<std::string> board{
      "#######################", "# #            ##     #",
      "# #  #  ######    ### #", "# #  #  #     #  #  # #",
      "# #  #  #  #  #  #  # #", "#    #     #     #    #",
      "##################  # #", "#                    ##",
      "#                    ##", "#  ####################",
      "#######################"};

  // ReplayDriver starts the agent at (1,1), and TrashInterface prints the
  // board before each move is applied.
  board[1][1] = '@';

  std::ostringstream out;
  out << '+' << std::string(board[0].size(), '-') << "+\n";
  for (const auto &row : board) {
    out << "|" << row << "|\n";
  }
  out << '+' << std::string(board[0].size(), '-') << "+\n";
  out << "\nUse W, A, S, D to move or Q to quit.";
  out << "\nYour move? ";

  return out.str();
}

} // namespace

TEST_CASE("ReplayDriver ReplayFullGame runs through valid actions",
          "[ReplayDriver]") {
  auto log = MakeFullGameLog();
  cse498::ReplayDriver driver(log, 0);

  REQUIRE_NOTHROW(CaptureStdout([&]() { driver.ReplayFullGame(); }));
}

TEST_CASE("ReplayDriver ReplayByTimeRange replays only selected time window",
          "[ReplayDriver]") {
  auto log = MakeTimedLog();
  cse498::ReplayDriver driver(log, 0);

  REQUIRE_NOTHROW(CaptureStdout([&]() { driver.ReplayByTimeRange(1.5, 2.5); }));
}

TEST_CASE("ReplayDriver ReplayByTimeRange handles empty ranges",
          "[ReplayDriver]") {
  auto log = MakeTimedLog();
  cse498::ReplayDriver driver(log, 0);

  REQUIRE_NOTHROW(
      CaptureStdout([&]() { driver.ReplayByTimeRange(10.0, 20.0); }));
}

TEST_CASE("ReplayDriver ReplayFullGame handles empty logs", "[ReplayDriver]") {
  auto log = std::make_shared<cse498::ActionLog>();
  cse498::ReplayDriver driver(log, 0);

  std::string output = CaptureStdout([&]() { driver.ReplayFullGame(); });

  REQUIRE(output.empty());
}

TEST_CASE("ReplayDriver SetActionLog replaces the current log",
          "[ReplayDriver]") {
  auto empty_log = std::make_shared<cse498::ActionLog>();
  auto full_log = MakeFullGameLog();

  cse498::ReplayDriver driver(empty_log, 0);

  std::string empty_output = CaptureStdout([&]() { driver.ReplayFullGame(); });
  REQUIRE(empty_output.empty());

  driver.SetActionLog(full_log);

  std::string full_output = CaptureStdout([&]() { driver.ReplayFullGame(); });
  REQUIRE_FALSE(full_output.empty());
}

TEST_CASE("ReplayDriver prints TrashInterface board and prompt",
          "[ReplayDriver][TrashInterface]") {
  auto log = MakeFullGameLog();
  cse498::ReplayDriver driver(log, 0);

  std::string output = CaptureStdout([&]() { driver.ReplayFullGame(); });

  CHECK(output.find(ExpectedStartingBoard()) != std::string::npos);
}