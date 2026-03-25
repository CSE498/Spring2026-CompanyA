#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/DataAnalytics/AnalyticsManager.hpp"

/*
Test checks all DataLogs are empty on creation
*/
TEST_CASE("Empty State Test", "[DataAnalytics]"){

    cse498::AnalyticsManager analytics;

    CHECK(analytics.GetHealthLog().Count() == 0);
    CHECK(analytics.GetEnemiesTrackedLog().Count() == 0);
    CHECK(analytics.GetDamageDealtLog().Count() == 0);

    CHECK_FALSE(analytics.GetHealthLog().Mean().has_value());
    CHECK_FALSE(analytics.GetEnemiesTrackedLog().Median().has_value());
    CHECK_FALSE(analytics.GetDamageDealtLog().Max().has_value());
    CHECK_FALSE(analytics.GetDamageDealtLog().Min().has_value());


}

/*
Test checks that the analytics manager stores and calculates the DataLogs correctly
*/
TEST_CASE("Populated Log Test", "[DataAnalytics]"){

    cse498::AnalyticsManager analytics;

    analytics.LogHealth(100.0);
    analytics.LogHealth(80.0);

    analytics.LogEnemiesTracked(5);
    analytics.LogEnemiesTracked(3);

    analytics.LogDamageDealt(50.0);
    analytics.LogDamageDealt(25.0);

    CHECK(analytics.GetHealthLog().Count() == 2);
    CHECK(analytics.GetEnemiesTrackedLog().Count() == 2);
    CHECK(analytics.GetDamageDealtLog().Count() == 2);

    CHECK(analytics.GetHealthLog().DataSamples()[0].value == 100.0);
    CHECK(analytics.GetEnemiesTrackedLog().DataSamples()[1].value == 3.0);
    CHECK(analytics.GetDamageDealtLog().DataSamples()[0].value == 50.0);

    CHECK(analytics.GetHealthLog().Mean().value() == 90.0);
    CHECK(analytics.GetEnemiesTrackedLog().Median().value() == 4.0);
    CHECK(analytics.GetDamageDealtLog().Max().value() == 50.0);
    CHECK(analytics.GetDamageDealtLog().Min().value() == 25.0);

}

/*
Test checks the Reset functionality of the analytics manager
*/
TEST_CASE("Reset Log Test", "[DataAnalytics]"){

    cse498::AnalyticsManager analytics;

    analytics.LogHealth(100.0);

    analytics.LogEnemiesTracked(5);

    analytics.LogDamageDealt(50.0);

    CHECK(analytics.GetHealthLog().Count() == 1);
    CHECK(analytics.GetEnemiesTrackedLog().Count() == 1);
    CHECK(analytics.GetDamageDealtLog().Count() == 1);

    analytics.Reset();

    CHECK(analytics.GetHealthLog().Count() == 0);
    CHECK(analytics.GetEnemiesTrackedLog().Count() == 0);
    CHECK(analytics.GetDamageDealtLog().Count() == 0);
}

//Tests for ActionLog will be added once ActionLog is implemented into this class