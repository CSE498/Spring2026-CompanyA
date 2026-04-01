#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/DataAnalytics/StatsTracker.hpp"

// StatsTracker
TEST_CASE("Build Series Summary", "[StatsTracker]") {
  cse498::StatsTracker tracker;
  cse498::DataLog log;
  REQUIRE(tracker.BuildSeriesSummary("test_key", "Test Key", log) ==
          std::nullopt);

  log.Add(4.5);
  log.Add(2.13);
  log.Add(43.07);

  cse498::StatSummary summary =
      tracker.BuildSeriesSummary("test_key", "Test Key", log).value();
  REQUIRE(summary.key == "test_key");
  REQUIRE(summary.label == "Test Key");
  REQUIRE(summary.sampleCount == 3);
  REQUIRE(summary.minValue == 2.13);
  REQUIRE(summary.maxValue == 43.07);
  REQUIRE(summary.meanValue == Approx(16.566).margin(0.001));
  REQUIRE(summary.medianValue == 4.5);
  REQUIRE(summary.currentValue == 43.07);
}

TEST_CASE("Build Snapshot", "[StatsTracker]") {
  cse498::StatsTracker tracker;
  cse498::AnalyticsManager analytics;
  cse498::DashboardSnapshot dashboard = tracker.BuildSnapshot(analytics);

  REQUIRE(dashboard.numericStats.size() == 0);
  REQUIRE(dashboard.actionStats.size() == 0);

  analytics.LogHealth(3.4);
  dashboard = tracker.BuildSnapshot(analytics);
  REQUIRE(dashboard.numericStats.size() == 1);

  analytics.LogDamageDealt(50);
  dashboard = tracker.BuildSnapshot(analytics);
  REQUIRE(dashboard.numericStats.size() == 2);

  analytics.LogHealth(5.2);
  dashboard = tracker.BuildSnapshot(analytics);
  REQUIRE(dashboard.numericStats.size() == 2);

  analytics.LogEnemiesTracked(2.2);
  dashboard = tracker.BuildSnapshot(analytics);
  REQUIRE(dashboard.numericStats.size() == 3);

  analytics.LogAction(1, "TestMove", cse498::WorldPosition(1, 2),
                      cse498::WorldPosition(2, 1));
  analytics.LogAction(1, "TestMove2", cse498::WorldPosition(2, 1),
                      cse498::WorldPosition(3, 3));
  analytics.LogAction(1, "TestMove3", cse498::WorldPosition(3, 3),
                      cse498::WorldPosition(4, 3));

  dashboard = tracker.BuildSnapshot(analytics);
  
  REQUIRE(dashboard.actionStats.size() == 1);
}

TEST_CASE("Build Action Summary", "[StatsTracker]") {
  cse498::StatsTracker tracker;
  cse498::ActionLog aLog;
  REQUIRE(tracker.BuildActionSummary("test_key", "Test Key", aLog) ==
          std::nullopt);

  aLog.LogAction(1, "TestMove", cse498::WorldPosition(1, 2),
                 cse498::WorldPosition(2, 1));
  aLog.LogAction(1, "TestMove2", cse498::WorldPosition(2, 1),
                 cse498::WorldPosition(3, 3));
  aLog.LogAction(1, "TestMove3", cse498::WorldPosition(3, 3),
                 cse498::WorldPosition(4, 3));

  cse498::ActionSummary summary =
      tracker.BuildActionSummary("test_key", "Test Key", aLog).value();
  REQUIRE(summary.key == "test_key");
  REQUIRE(summary.label == "Test Key");
  REQUIRE(summary.actionCount == 3);
  REQUIRE(summary.mostActiveEntity == 1);
}
