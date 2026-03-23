/**
 * @file ActionLogTest.cpp
 * @brief Catch2 unit tests for ActionLog, AgentActionLog, and UserActionLog.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/ActionLog.hpp"

#include <string>
#include <vector>

// Helpers

/// Populates a log with `count` sequential move actions for a single entity.
static void FillLog(cse498::ActionLog& log, int entityId, int count) {
    for (int i = 0; i < count; ++i) {
        log.UpdateTime(static_cast<double>(i));
        log.LogAction(entityId, "move",
                      static_cast<double>(i), 0.0,
                      static_cast<double>(i + 1), 0.0);
    }
}

// ActionLog — initial state

TEST_CASE("ActionLog initial state is empty", "[actionlog][init]")
{
    cse498::ActionLog log;

    CHECK(log.GetActionCount() == 0);
    CHECK(log.GetActions().empty());
}

// ActionLog — LogAction / GetActions

TEST_CASE("ActionLog records actions with correct fields", "[actionlog][log]")
{
    cse498::ActionLog log;

    log.UpdateTime(1.0);
    log.LogAction(1, "move", 0.0, 0.0, 5.0, 5.0);

    log.UpdateTime(2.0);
    log.LogAction(2, "attack", 1.0, 1.0, 6.0, 6.0);

    REQUIRE(log.GetActionCount() == 2);

    const auto& actions = log.GetActions();

    SECTION("First action fields are correct")
    {
        CHECK(actions[0].EntityId       == 1);
        CHECK(actions[0].ActionType     == "move");
        CHECK(actions[0].Timestamp      == 1.0);
        CHECK(actions[0].X              == 0.0);
        CHECK(actions[0].Y              == 0.0);
        CHECK(actions[0].NewX           == 5.0);
        CHECK(actions[0].NewY           == 5.0);
        CHECK(actions[0].SequenceNumber == 0);
    }

    SECTION("Second action fields are correct")
    {
        CHECK(actions[1].EntityId       == 2);
        CHECK(actions[1].ActionType     == "attack");
        CHECK(actions[1].Timestamp      == 2.0);
        CHECK(actions[1].SequenceNumber == 1);
    }
}

// ActionLog — GetEntityActions

TEST_CASE("ActionLog::GetEntityActions filters by entity ID", "[actionlog][getentity]")
{
    cse498::ActionLog log;

    log.UpdateTime(1.0);
    log.LogAction(1, "move",   0.0, 0.0, 1.0, 0.0);
    log.LogAction(2, "move",   5.0, 5.0, 6.0, 5.0);
    log.LogAction(1, "attack", 1.0, 0.0, 1.0, 0.0);

    SECTION("Entity 1 has two actions")
    {
        auto result = log.GetEntityActions(1);
        REQUIRE(result.size() == 2);
        CHECK(result[0].ActionType == "move");
        CHECK(result[1].ActionType == "attack");
    }

    SECTION("Entity 2 has one action")
    {
        auto result = log.GetEntityActions(2);
        REQUIRE(result.size() == 1);
        CHECK(result[0].EntityId == 2);
    }

    SECTION("Unknown entity returns empty list")
    {
        CHECK(log.GetEntityActions(99).empty());
    }
}

// ActionLog — GetActionRange

TEST_CASE("ActionLog::GetActionRange filters by time range", "[actionlog][range]")
{
    cse498::ActionLog log;

    log.UpdateTime(1.0); log.LogAction(1, "move", 0, 0, 1, 0);
    log.UpdateTime(2.0); log.LogAction(1, "move", 1, 0, 2, 0);
    log.UpdateTime(3.0); log.LogAction(1, "move", 2, 0, 3, 0);

    SECTION("Exact range returns matching actions")
    {
        auto result = log.GetActionRange(1.5, 3.0);
        REQUIRE(result.size() == 2);
        CHECK(result[0].Timestamp == 2.0);
        CHECK(result[1].Timestamp == 3.0);
    }

    SECTION("Inclusive boundaries include endpoints")
    {
        auto result = log.GetActionRange(1.0, 1.0);
        REQUIRE(result.size() == 1);
        CHECK(result[0].Timestamp == 1.0);
    }

    SECTION("Range with no matches returns empty")
    {
        CHECK(log.GetActionRange(5.0, 10.0).empty());
    }
}

// ActionLog — Clear

TEST_CASE("ActionLog::Clear resets action list and sequence counter", "[actionlog][clear]")
{
    cse498::ActionLog log;

    log.UpdateTime(1.0);
    log.LogAction(1, "move", 0, 0, 1, 0);
    log.LogAction(1, "move", 1, 0, 2, 0);
    REQUIRE(log.GetActionCount() == 2);

    log.Clear();
    CHECK(log.GetActionCount() == 0);
    CHECK(log.GetActions().empty());

    // Sequence numbers should restart from 0 after a clear
    log.LogAction(1, "move", 0, 0, 1, 0);
    CHECK(log.GetActions()[0].SequenceNumber == 0);
}

// ActionLog — IsEntityStuck

TEST_CASE("ActionLog::IsEntityStuck detects stationary entities", "[actionlog][stuck]")
{
    cse498::ActionLog log;

    SECTION("Entity that never moves is stuck")
    {
        for (int i = 0; i < 5; ++i) {
            log.UpdateTime(static_cast<double>(i));
            log.LogAction(1, "idle", 3.0, 3.0, 3.0, 3.0); // no displacement
        }
        CHECK(log.IsEntityStuck(1, 5));
    }

    SECTION("Entity that moves is not stuck")
    {
        for (int i = 0; i < 5; ++i) {
            log.UpdateTime(static_cast<double>(i));
            log.LogAction(1, "move",
                          static_cast<double>(i), 0.0,
                          static_cast<double>(i + 1), 0.0);
        }
        CHECK_FALSE(log.IsEntityStuck(1, 5));
    }

    SECTION("Fewer actions than window size returns false")
    {
        log.UpdateTime(0.0);
        log.LogAction(1, "idle", 0.0, 0.0, 0.0, 0.0);
        CHECK_FALSE(log.IsEntityStuck(1, 5));
    }

    SECTION("Unknown entity returns false")
    {
        CHECK_FALSE(log.IsEntityStuck(99, 3));
    }
}

// ActionLog — Serialize / Deserialize

TEST_CASE("ActionLog serialize and deserialize round-trips correctly", "[actionlog][serialize]")
{
    cse498::ActionLog original;

    original.UpdateTime(1.0);
    original.LogAction(1, "move",   0.0, 0.0, 5.0, 5.0);
    original.UpdateTime(2.0);
    original.LogAction(2, "attack", 5.0, 5.0, 5.0, 5.0);

    std::string data = original.Serialize();

    cse498::ActionLog restored;
    restored.Deserialize(data);

    REQUIRE(restored.GetActionCount() == 2);

    const auto& actions = restored.GetActions();
    CHECK(actions[0].EntityId   == 1);
    CHECK(actions[0].ActionType == "move");
    CHECK(actions[0].Timestamp  == 1.0);
    CHECK(actions[1].EntityId   == 2);
    CHECK(actions[1].ActionType == "attack");
}

// AgentActionLog — GetStuckAgentRatio

TEST_CASE("AgentActionLog::GetStuckAgentRatio reports correct proportion", "[agentlog][stuck]")
{
    cse498::AgentActionLog log;

    // Agent 1: stationary for 5 actions
    for (int i = 0; i < 5; ++i) {
        log.UpdateTime(static_cast<double>(i));
        log.LogAction(1, "idle", 2.0, 2.0, 2.0, 2.0);
    }

    // Agent 2: moves each action
    for (int i = 0; i < 5; ++i) {
        log.UpdateTime(static_cast<double>(i));
        log.LogAction(2, "move",
                      static_cast<double>(i), 0.0,
                      static_cast<double>(i + 1), 0.0);
    }

    SECTION("Half the agents are stuck")
    {
        double ratio = log.GetStuckAgentRatio(5);
        CHECK(ratio == Approx(0.5));
    }

    SECTION("Empty log returns 0.0")
    {
        cse498::AgentActionLog empty;
        CHECK(empty.GetStuckAgentRatio(5) == Approx(0.0));
    }
}

// UserActionLog — GetLastAction / GetMostFrequentActionType

TEST_CASE("UserActionLog::GetLastAction returns the most recent entry", "[userlog][last]")
{
    cse498::UserActionLog log;

    SECTION("Empty log returns nullopt")
    {
        CHECK_FALSE(log.GetLastAction().has_value());
    }

    SECTION("Returns the last logged action")
    {
        log.UpdateTime(1.0); log.LogAction(0, "jump", 0, 0, 0, 1);
        log.UpdateTime(2.0); log.LogAction(0, "run",  0, 1, 5, 1);

        auto last = log.GetLastAction();
        REQUIRE(last.has_value());
        CHECK(last->ActionType == "run");
        CHECK(last->Timestamp  == 2.0);
    }
}

TEST_CASE("UserActionLog::GetMostFrequentActionType identifies dominant action", "[userlog][freq]")
{
    cse498::UserActionLog log;

    SECTION("Empty log returns nullopt")
    {
        CHECK_FALSE(log.GetMostFrequentActionType().has_value());
    }

    SECTION("Returns the most common action type")
    {
        log.UpdateTime(0.0); log.LogAction(0, "move",  0, 0, 1, 0);
        log.UpdateTime(1.0); log.LogAction(0, "move",  1, 0, 2, 0);
        log.UpdateTime(2.0); log.LogAction(0, "jump",  2, 0, 2, 1);
        log.UpdateTime(3.0); log.LogAction(0, "move",  2, 1, 3, 1);

        auto freq = log.GetMostFrequentActionType();
        REQUIRE(freq.has_value());
        CHECK(*freq == "move");
    }
}

// Stress test

TEST_CASE("ActionLog handles a large number of actions correctly", "[actionlog][stress]")
{
    static constexpr int ACTION_COUNT = 10'000;

    cse498::ActionLog log;
    FillLog(log, 1, ACTION_COUNT);

    SECTION("All actions are recorded")
    {
        CHECK(log.GetActionCount() == ACTION_COUNT);
    }

    SECTION("Sequence numbers are monotonically increasing")
    {
        const auto& actions = log.GetActions();
        for (int i = 0; i < ACTION_COUNT; ++i) {
            CHECK(actions[i].SequenceNumber == i);
        }
    }

    SECTION("GetEntityActions returns all entries for a single entity")
    {
        CHECK(static_cast<int>(log.GetEntityActions(1).size()) == ACTION_COUNT);
    }

    SECTION("GetActionRange across full range returns everything")
    {
        auto result = log.GetActionRange(0.0, static_cast<double>(ACTION_COUNT - 1));
        CHECK(static_cast<int>(result.size()) == ACTION_COUNT);
    }

    SECTION("Clear empties a large log")
    {
        log.Clear();
        CHECK(log.GetActionCount() == 0);
    }
}
