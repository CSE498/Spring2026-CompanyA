#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/AnalyticsManager.hpp"

/*
Test checks all DataLogs are empty on creation
*/
TEST_CASE("Empty State Test", "[tools]"){

    cse498::AnalyticsManager analytics;

    CHECK(analytics.GetSuccessfulMovesLog().Count() == 0);
    CHECK(analytics.GetBlockedMovesLog().Count() == 0);
    CHECK(analytics.GetHealthLog().Count() == 0);
    CHECK(analytics.GetEnemiesTrackedLog().Count() == 0);
    CHECK(analytics.GetDamageDealtLog().Count() == 0);

    CHECK_FALSE(analytics.GetSuccessfulMovesLog().Min().has_value());
    CHECK_FALSE(analytics.GetBlockedMovesLog().Max().has_value());
    CHECK_FALSE(analytics.GetHealthLog().Mean().has_value());
    CHECK_FALSE(analytics.GetEnemiesTrackedLog().Median().has_value());
}

/*
Test checks that the analytics manager stores and calculates the DataLogs correctly
*/
TEST_CASE("Populated Log Test", "[tools]"){

    cse498::AnalyticsManager analytics;

    analytics.LogSuccessfulMove(1.0);
    analytics.LogSuccessfulMove(1.0);

    analytics.LogBlockedMove(-1.0);
    analytics.LogBlockedMove(-1.0);

    analytics.LogHealth(100.0);
    analytics.LogHealth(80.0);

    analytics.LogEnemiesTracked(5);
    analytics.LogEnemiesTracked(3);

    analytics.LogDamageDealt(50.0);
    analytics.LogDamageDealt(25.0);

    CHECK(analytics.GetSuccessfulMovesLog().Count() == 2);
    CHECK(analytics.GetBlockedMovesLog().Count() == 2);
    CHECK(analytics.GetHealthLog().Count() == 2);
    CHECK(analytics.GetEnemiesTrackedLog().Count() == 2);
    CHECK(analytics.GetDamageDealtLog().Count() == 2);

    CHECK(analytics.GetSuccessfulMovesLog().DataSamples()[0].value == 1.0);
    CHECK(analytics.GetBlockedMovesLog().DataSamples()[1].value == -1.0);
    CHECK(analytics.GetHealthLog().DataSamples()[0].value == 100.0);
    CHECK(analytics.GetEnemiesTrackedLog().DataSamples()[1].value == 3.0);
    CHECK(analytics.GetDamageDealtLog().DataSamples()[0].value == 50.0);

    CHECK(analytics.GetHealthLog().Mean().value() == 90.0);
    CHECK(analytics.GetEnemiesTrackedLog().Median().value() == 4.0);
    CHECK(analytics.GetDamageDealtLog().Max().value() == 50.0);
}

/*
Test checks the Reset functionality of the analytics manager
*/
TEST_CASE("Reset Log Test", "[tools]"){

    cse498::AnalyticsManager analytics;

    analytics.LogSuccessfulMove(1.0);

    analytics.LogBlockedMove(-1.0);

    analytics.LogHealth(100.0);

    analytics.LogEnemiesTracked(5);

    analytics.LogDamageDealt(50.0);

    CHECK(analytics.GetSuccessfulMovesLog().Count() == 1);
    CHECK(analytics.GetBlockedMovesLog().Count() == 1);
    CHECK(analytics.GetHealthLog().Count() == 1);
    CHECK(analytics.GetEnemiesTrackedLog().Count() == 1);
    CHECK(analytics.GetDamageDealtLog().Count() == 1);

    analytics.Reset();

    CHECK(analytics.GetSuccessfulMovesLog().Count() == 0);
    CHECK(analytics.GetBlockedMovesLog().Count() == 0);
    CHECK(analytics.GetHealthLog().Count() == 0);
    CHECK(analytics.GetEnemiesTrackedLog().Count() == 0);
    CHECK(analytics.GetDamageDealtLog().Count() == 0);
}