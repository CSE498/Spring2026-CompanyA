/**
 * @file StatsTracker.cpp
 * @brief Implementation of StatsTracker and related dashboard summary helpers.
 * used AI to help in formatting the display data function
 */

#include "StatsTracker.hpp"
#include <iostream>

namespace cse498 {

// StatsTracker

/**
 * Builds a summary for one numeric series.
 * @param key Stable identifier for the series.
 * @param label Human-readable name for dashboard display.
 * @param log Existing DataLog from the analytics layer.
 * @return Summary data if the series contains samples, otherwise std::nullopt.
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

/**
 * Builds a full dashboard snapshot of all tracked data.
 * @param analytics Analytics manager that owns the underlying logs.
 * @return Combined snapshot of numeric series summaries and action summaries.
 */
DashboardSnapshot StatsTracker::BuildSnapshot(const AnalyticsManager& analytics) const {
    DashboardSnapshot snapshot;

    if (auto summary = BuildSeriesSummary("enemies_killed", "Enemies Killed", analytics.GetEnemiesKilledLog());
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

/**
 * Builds a summary for one action log.
 * @param key Stable identifier for the action summary.
 * @param label Human-readable name for dashboard display.
 * @param log Existing ActionLog from the analytics layer.
 * @return Summary data if the log contains actions, otherwise std::nullopt.
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

/**
 * Prepares and outputs dashboard data through the analytics manager.
 * Used as a temporary way to output the summaries to the console until the GUI is implemented.
 * @param Analytics manager that owns the underlying logs.
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
