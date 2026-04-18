/**
 * @file StatsTracker.cpp
 * @brief Implementation of StatsTracker and related dashboard summary helpers.
 * used AI to help in formatting the display data function
 */

#include "StatsTracker.hpp"
#include <iostream>

namespace cse498 {

// StatsTracker

/*
 * Builds the dashboard-facing summary for one numeric series.
 */
std::optional<StatSummary> StatsTracker::BuildSeriesSummary(const std::string& key, const std::string& label,
                                                            const DataLog& log) const {
    if (log.Count() == 0) {
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
    if (!samples.empty()) {
        summary.currentValue = samples.back().value;
    }

    return summary;
}

/*
 * Combines all numeric series summaries and counters into one snapshot payload.
 */
DashboardSnapshot StatsTracker::BuildSnapshot(const AnalyticsManager& analytics) const {
    DashboardSnapshot snapshot;


    if (auto summary = BuildSeriesSummary("health", "Health", analytics.GetHealthLog()); summary.has_value()) {
        snapshot.numericStats.push_back(*summary);
    }

    if (auto summary = BuildSeriesSummary("enemies_tracked", "Enemies Tracked", analytics.GetEnemiesTrackedLog());
        summary.has_value()) {
        snapshot.numericStats.push_back(*summary);
    }

    if (auto summary = BuildSeriesSummary("damage_dealt", "Damage Dealt", analytics.GetDamageDealtLog());
        summary.has_value()) {
        snapshot.numericStats.push_back(*summary);
    }

    if (auto summary = BuildActionSummary("action_log", "Action Log", analytics.GetActionLog()); summary.has_value()) {
        snapshot.actionStats.push_back(*summary);
    }

    return snapshot;
}

/*
 * Builds the dashboard-facing summary for one action log.
 */
std::optional<ActionSummary> StatsTracker::BuildActionSummary(const std::string& key, const std::string& label,
                                                              const ActionLog& log) const {
    if (log.GetActionCount() == 0) {
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
 * Prepares and outputs dashboard data through the analytics manager.
 * Used as a temporary way to output the summaries to the console until the GUI is implemented.
 */
void StatsTracker::DisplayData(const AnalyticsManager& analytics) const{
    auto snapshot = BuildSnapshot(analytics);

    std::cout << "========== Dashboard Snapshot ==========" << "\n";

    if(snapshot.numericStats.empty() && snapshot.actionStats.empty()) {
        std::cout << "No data to display" << "\n";
        return;
    }

    std::cout << "DataLog Stats:" << "\n";
    for (const auto& stat: snapshot.numericStats) {
        std::cout << "----------------------------------------" << "\n";
        std::cout << "- " << stat.label << "\n";

        std::cout << "Current Value: " << stat.currentValue << "\n";

        std::cout << "Min Value: ";
        if (stat.minValue.has_value()) {
            std::cout << *stat.minValue << "\n";
        } 
        else {
            std::cout << "N/A" << "\n";
        }

        std::cout << "Max Value: ";
        if (stat.maxValue.has_value()) {
            std::cout << *stat.maxValue << "\n";
        } 
        else {
            std::cout << "N/A" << "\n";
        }
        std::cout << "Mean Value: ";
        if (stat.meanValue.has_value()) {
            std::cout << *stat.meanValue << "\n";
        } 
        else {
            std::cout << "N/A" << "\n";
        }

        std::cout << "Median Value: ";
        if (stat.medianValue.has_value()) {
            std::cout << *stat.medianValue << "\n";
        } 
        else {
            std::cout << "N/A" << "\n";
        }
        
        std::cout << "Sample Count: " << stat.sampleCount << "\n";
        std::cout << "----------------------------------------" << "\n";
    }

    std::cout << "ActionLog Stats:" << "\n";
    for(const auto& actionStat: snapshot.actionStats) {
        std::cout << "----------------------------------------" << "\n";
        std::cout << "Label: " << actionStat.label << "\n";
        std::cout << "Action Count: " << actionStat.actionCount << "\n";
        std::cout << "Most Active Entity: ";
        if (actionStat.mostActiveEntity.has_value()) {
            std::cout << *actionStat.mostActiveEntity << "\n";
        } 
        else {
            std::cout << "N/A" << "\n";
        }
        std::cout << "----------------------------------------" << "\n";
    }

    std::cout << "========================================" << "\n";
}
} // namespace cse498
