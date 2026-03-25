/**
 * @file StatsTracker.cpp
 * @brief Implementation of StatsTracker and related dashboard summary helpers.
 */

#include "StatsTracker.hpp"

namespace cse498 
{

// StatsTracker

/*
* Builds the dashboard-facing summary for one numeric series.
*/
std::optional<StatSummary> StatsTracker::BuildSeriesSummary(const std::string& key, const std::string& label, const DataLog& log) const 
{
    if (log.Count() == 0) 
    {
        return std::nullopt;
    }

    StatSummary summary;
    summary.key = key;
    summary.label = label;
    summary.sampleCount = log.Count();
    summary.minValue = log.Min();
    summary.maxValue = log.Max();
    summary.meanValue = log.Mean();
    summary.medianValue = log.Median();

    const auto& samples = log.DataSamples();
    if (!samples.empty()) 
    {
        summary.currentValue = samples.back().value;
    }

    return summary;
}

/*
* Combines all numeric series summaries and counters into one snapshot payload.
*/
DashboardSnapshot StatsTracker::BuildSnapshot(const AnalyticsManager& analytics) const 
{
    DashboardSnapshot snapshot;

    if (auto summary = BuildSeriesSummary("successful_moves", "Successful Moves", analytics.GetSuccessfulMovesLog()); summary.has_value()) 
    {
        snapshot.numericStats.push_back(*summary);
    }

    if (auto summary = BuildSeriesSummary("blocked_moves", "Blocked Moves", analytics.GetBlockedMovesLog()); summary.has_value()) 
    {
        snapshot.numericStats.push_back(*summary);
    }

    if (auto summary = BuildSeriesSummary("health", "Health", analytics.GetHealthLog()); summary.has_value()) 
    {
        snapshot.numericStats.push_back(*summary);
    }

    if (auto summary = BuildSeriesSummary("enemies_tracked", "Enemies Tracked", analytics.GetEnemiesTrackedLog()); summary.has_value()) 
    {
        snapshot.numericStats.push_back(*summary);
    }

    if (auto summary = BuildSeriesSummary("damage_dealt", "Damage Dealt", analytics.GetDamageDealtLog()); summary.has_value()) 
    {
        snapshot.numericStats.push_back(*summary);
    }

    return snapshot;
}

/*
* Builds the dashboard-facing summary for one action log.
*/
std::optional<ActionSummary> StatsTracker::BuildActionSummary(const std::string& key, const std::string& label, const ActionLog& log) const
{
    if (log.GetActionCount() == 0)
    {
        return std::nullopt;
    }

    ActionSummary summary;
    summary.key = key;
    summary.label = label;
    summary.actionCount = log.GetActionCount();
    summary.mostActiveEntity = log.GetMostActiveEntity();

    return summary;
}

/*
* Combines numeric summaries and one action summary into one snapshot payload.
*/
DashboardSnapshot StatsTracker::BuildSnapshot(const AnalyticsManager& analytics, const ActionLog& actions) const
{
    DashboardSnapshot snapshot = BuildSnapshot(analytics);

    if (auto summary = BuildActionSummary("action_log", "Action Log", actions); summary.has_value())
    {
        snapshot.actionStats.push_back(*summary);
    }

    return snapshot;
}

}
