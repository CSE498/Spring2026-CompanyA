#include "../../source/Analyze/AnalyticsManager.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

/*
Test checks all DataLogs are empty on creation
*/
TEST_CASE("Empty State Test", "[DataAnalytics]") {

    cse498::AnalyticsManager analytics;

    CHECK(analytics.GetEnemiesKilledLog().Count() == 0);
    CHECK(analytics.GetDamageDealtLog().Count() == 0);

    CHECK_FALSE(analytics.GetEnemiesKilledLog().Median().has_value());
    CHECK_FALSE(analytics.GetDamageDealtLog().Max().has_value());
    CHECK_FALSE(analytics.GetDamageDealtLog().Min().has_value());
}

/*
Test checks that the analytics manager stores and calculates the DataLogs
correctly
*/
TEST_CASE("Populated Log Test", "[DataAnalytics]") {

    cse498::AnalyticsManager analytics;

    analytics.LogEnemiesKilled(5);
    analytics.LogEnemiesKilled(3);

    analytics.LogDamageDealt(50.0);
    analytics.LogDamageDealt(25.0);

    CHECK(analytics.GetEnemiesKilledLog().Count() == 2);
    CHECK(analytics.GetDamageDealtLog().Count() == 2);

    CHECK(analytics.GetEnemiesKilledLog().DataSamples()[1].value == 3.0);
    CHECK(analytics.GetDamageDealtLog().DataSamples()[0].value == 50.0);

    CHECK(analytics.GetEnemiesKilledLog().Median().value() == 4.0);
    CHECK(analytics.GetDamageDealtLog().Max().value() == 50.0);
    CHECK(analytics.GetDamageDealtLog().Min().value() == 25.0);
}

/*
Test checks the Reset functionality of the analytics manager
*/
TEST_CASE("Reset Log Test", "[DataAnalytics]") {

    cse498::AnalyticsManager analytics;

    analytics.LogEnemiesKilled(5);

    analytics.LogDamageDealt(50.0);

    CHECK(analytics.GetEnemiesKilledLog().Count() == 1);
    CHECK(analytics.GetDamageDealtLog().Count() == 1);

    analytics.ResetEnemiesKilledLog();
    analytics.ResetDamageDealtLog();

    CHECK(analytics.GetEnemiesKilledLog().Count() == 0);
    CHECK(analytics.GetDamageDealtLog().Count() == 0);

    analytics.LogEnemiesKilled(2);

    analytics.LogDamageDealt(50.0);

    CHECK(analytics.GetEnemiesKilledLog().Count() == 1);
    CHECK(analytics.GetDamageDealtLog().Count() == 1);

    analytics.Reset();

    CHECK(analytics.GetEnemiesKilledLog().Count() == 0);
    CHECK(analytics.GetDamageDealtLog().Count() == 0);
}


/*
Test checks that negative values are not added to the DataLogs
*/
TEST_CASE("Negative Value Test", "[DataAnalytics]") {

    cse498::AnalyticsManager analytics;

    analytics.LogEnemiesKilled(-5);

    analytics.LogDamageDealt(-50.0);

    CHECK(analytics.GetEnemiesKilledLog().Count() == 0);
    CHECK(analytics.GetDamageDealtLog().Count() == 0);
}

TEST_CASE("AnalyticsManager ActionLog empty on construction", "[DataAnalytics][actionlog]") {
    cse498::AnalyticsManager analytics;

    CHECK(analytics.GetActionLog().GetActionCount() == 0);
    CHECK(analytics.GetActionLog().GetActions().empty());
}

TEST_CASE("AnalyticsManager LogAction stores actions correctly", "[DataAnalytics][actionlog]") {
    cse498::AnalyticsManager analytics;

    analytics.UpdateActionTime(1.0);
    analytics.LogAction(1, "move", {0.0, 0.0}, {1.0, 0.0});

    REQUIRE(analytics.GetActionLog().GetActionCount() == 1);

    const auto& actions = analytics.GetActionLog().GetActions();
    CHECK(actions[0].EntityId == 1);
    CHECK(actions[0].ActionType == "move");
    CHECK(actions[0].Timestamp == 1.0);
    CHECK(actions[0].Position.X() == 0.0);
    CHECK(actions[0].Position.Y() == 0.0);
    CHECK(actions[0].NewPosition.X() == 1.0);
    CHECK(actions[0].NewPosition.Y() == 0.0);
}

TEST_CASE("AnalyticsManager LogAction records multiple actions", "[DataAnalytics][actionlog]") {
    cse498::AnalyticsManager analytics;

    analytics.UpdateActionTime(1.0);
    analytics.LogAction(1, "move", {0.0, 0.0}, {1.0, 0.0});
    analytics.UpdateActionTime(2.0);
    analytics.LogAction(2, "attack", {1.0, 0.0}, {1.0, 0.0});
    analytics.UpdateActionTime(3.0);
    analytics.LogAction(1, "move", {1.0, 0.0}, {2.0, 0.0});

    CHECK(analytics.GetActionLog().GetActionCount() == 3);
    CHECK(analytics.GetActionLog().GetEntityActions(1).size() == 2);
    CHECK(analytics.GetActionLog().GetEntityActions(2).size() == 1);
}

TEST_CASE("AnalyticsManager Reset clears action log", "[DataAnalytics][actionlog]") {
    cse498::AnalyticsManager analytics;

    analytics.UpdateActionTime(1.0);
    analytics.LogAction(1, "move", {0.0, 0.0}, {1.0, 0.0});
    REQUIRE(analytics.GetActionLog().GetActionCount() == 1);

    analytics.ResetActionLog();
    CHECK(analytics.GetActionLog().GetActionCount() == 0);
    CHECK(analytics.GetActionLog().GetActions().empty());
}
