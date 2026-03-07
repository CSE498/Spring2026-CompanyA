// #define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/ActionLog.hpp"
#include "../../source/tools/ReplayDriver.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <memory>

// static std::vector<std::string> ReadAllLines(const std::string& filename) {
//     std::ifstream in(filename);
//     std::vector<std::string> lines;
//     std::string line;
//     while (std::getline(in, line)) lines.push_back(line);
//     return lines;
// }

// Build a log with known timestamps via UpdateTime(), and known action types/agents.
static std::shared_ptr<cse498::ActionLog> MakeSampleLog() {
    std::shared_ptr<cse498::ActionLog> log = std::make_shared<cse498::ActionLog>();

    log->UpdateTime(0.0);
    log->LogAction(1, "move",   0.0, 0.0, 1.0, 1.0); // (x,y,new_x,new_y)

    log->UpdateTime(1.0);
    log->LogAction(2, "attack", 1.0, 1.0, 1.0, 1.0);

    log->UpdateTime(2.0);
    log->LogAction(1, "move",   2.0, 2.0, 3.0, 3.0);

    log->UpdateTime(3.0);
    log->LogAction(2, "move",   3.0, 3.0, 4.0, 4.0);

    return log;
}

// Tests for ReplayDriver
TEST_CASE("ReplayDriver: IsActionLogSet()", "[ReplayDriver]") {
    cse498::ReplayDriver driver;
    CHECK(driver.IsActionLogSet() == false);

    std::shared_ptr<cse498::ActionLog> log = MakeSampleLog();
    driver.setActionLog(log);
    CHECK(driver.IsActionLogSet() == true);
}

//Empty replays to csv
TEST_CASE("ReplayDriver: SaveReplayToFile returns false if no log set", "[ReplayDriver]") {
    cse498::ReplayDriver driver;
    CHECK(driver.SaveReplayToFile("should_not_write.csv") == false);
}

// //Full Replays to csv
// TEST_CASE("ReplayDriver: Replay writes all actions to CSV", "[ReplayDriver]") {
//     std::shared_ptr<cse498::ActionLog> log = MakeSampleLog();
//     cse498::ReplayDriver driver(log);

//     driver.Replay();

//     const std::string file = "replay_all.csv";
//     REQUIRE(driver.SaveReplayToFile(file) == true);

//     auto lines = ReadAllLines(file);
//     REQUIRE(lines.size() == 4);

//     CHECK(lines[0].rfind("1,move,",   0) == 0);
//     CHECK(lines[1].rfind("2,attack,", 0) == 0);
//     CHECK(lines[2].rfind("1,move,",   0) == 0);
//     CHECK(lines[3].rfind("2,move,",   0) == 0);
// }

// //Replays by steps
// TEST_CASE("ReplayDriver: ReplayByStep selects every Nth action", "[ReplayDriver]") {
//     std::shared_ptr<cse498::ActionLog> log = MakeSampleLog();
//     cse498::ReplayDriver driver(log);

//     driver.ReplayByStep(2);

//     const std::string file = "replay_step2.csv";
//     REQUIRE(driver.SaveReplayToFile(file) == true);

//     auto lines = ReadAllLines(file);

//     REQUIRE(lines.size() == 2);
//     CHECK(lines[0].rfind("1,move,", 0) == 0);
//     CHECK(lines[1].rfind("1,move,", 0) == 0);
// }

// // Replays by steps with step <= 0 should replay nothing (safe)
// TEST_CASE("ReplayDriver: ReplayByStep with step <= 0 replays nothing (safe)", "[ReplayDriver]") {
//     std::shared_ptr<cse498::ActionLog> log = MakeSampleLog();
//     cse498::ReplayDriver driver(log);

//     driver.ReplayByStep(0);

//     const std::string file = "replay_step0.csv";
//     REQUIRE(driver.SaveReplayToFile(file) == true);

//     auto lines = ReadAllLines(file);
//     CHECK(lines.empty());
// }

// // Replays by time range
// TEST_CASE("ReplayDriver: ReplayByTimeRange filters actions by timestamp", "[ReplayDriver]") {
//     std::shared_ptr<cse498::ActionLog> log = MakeSampleLog();
//     cse498::ReplayDriver driver(log);

//     driver.ReplayByTimeRange(1.0, 2.0);

//     const std::string file = "replay_timerange.csv";
//     REQUIRE(driver.SaveReplayToFile(file) == true);

//     auto lines = ReadAllLines(file);
//     REQUIRE(lines.size() == 2);
//     CHECK(lines[0].rfind("2,attack,", 0) == 0);
//     CHECK(lines[1].rfind("1,move,",   0) == 0);
// }

// // Replays by agent
// TEST_CASE("ReplayDriver: ReplayByAgent filters actions", "[ReplayDriver]") {
//     std::shared_ptr<cse498::ActionLog> log = MakeSampleLog();
//     cse498::ReplayDriver driver(log);

//     driver.ReplayByAgent(2);

//     const std::string file = "replay_agent2.csv";
//     REQUIRE(driver.SaveReplayToFile(file) == true);

//     auto lines = ReadAllLines(file);
//     REQUIRE(lines.size() == 2);
//     CHECK(lines[0].rfind("2,attack,", 0) == 0);
//     CHECK(lines[1].rfind("2,move,",   0) == 0);
// }

// // Replays by action type
// TEST_CASE("ReplayDriver: ReplayByActionType filters actions", "[ReplayDriver]") {
//     std::shared_ptr<cse498::ActionLog> log = MakeSampleLog();
//     cse498::ReplayDriver driver(log);

//     driver.ReplayByActionType("move");

//     const std::string file = "replay_move.csv";
//     REQUIRE(driver.SaveReplayToFile(file) == true);

//     auto lines = ReadAllLines(file);
//     REQUIRE(lines.size() == 3);
//     CHECK(lines[0].rfind("1,move,", 0) == 0);
//     CHECK(lines[1].rfind("1,move,", 0) == 0);
//     CHECK(lines[2].rfind("2,move,", 0) == 0);
// }

// //Replays are replaced after each new replay method is called
// TEST_CASE("ReplayDriver: A new replay replaces the last replayed set", "[ReplayDriver]") {
//     std::shared_ptr<cse498::ActionLog> log = MakeSampleLog();
//     cse498::ReplayDriver driver(log);

//     driver.ReplayByAgent(1);
//     REQUIRE(driver.SaveReplayToFile("tmp_agent1.csv") == true);
//     auto agent1 = ReadAllLines("tmp_agent1.csv");
//     REQUIRE(agent1.size() == 2);

//     driver.ReplayByActionType("attack");
//     REQUIRE(driver.SaveReplayToFile("tmp_attack.csv") == true);
//     auto attack = ReadAllLines("tmp_attack.csv");
//     REQUIRE(attack.size() == 1);
//     CHECK(attack[0].rfind("2,attack,", 0) == 0);
// }
