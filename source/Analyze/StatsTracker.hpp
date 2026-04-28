/**
 * @file StatsTracker.hpp
 * @author Tyler Murray, Nathan Charters
 * @brief Builds dashboard summaries from AnalyticsManager logs.
 *
 * StatsTracker reads existing logs from AnalyticsManager and produces a
 * GUI-friendly snapshot containing stable keys, display labels, and summary
 * statistics for each tracked series.
 */

#pragma once

#include "../tools/ActionLog.hpp"
#include "../tools/DataLog.hpp"
#include "AnalyticsManager.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <iomanip>


namespace cse498 {

/**
 * @struct StatSummary
 * Summary information for one numeric statistic series.
 * This structure is intended for dashboard consumption. The key is the stable
 * programmatic identifier, while the label is human-readable display text.
 */
struct StatSummary {
    std::string key; // Stable identifier for the statistic, e.g., "successful_moves"
    std::string label; // For example, "Successful Moves"
    double currentValue = 0.0;
    std::optional<double> minValue;
    std::optional<double> maxValue;
    std::optional<double> meanValue;
    std::optional<double> medianValue;
    std::size_t sampleCount = 0;
};

 /**
  * @struct ActionSummary
  * Summary information derived from an action log.
  */
struct ActionSummary {
    std::string key; // Stable identifier for the action summary, e.g.,
                     // "most_active_entity"
    std::string label; // For example, "Most Active Entity" or "Total Actions"
    int actionCount = 0;
    std::optional<int> mostActiveEntity;
};

 /**
  * @struct DashboardSnapshot
  * Combined dashboard payload produced by StatsTracker.
  * Numeric summaries and action summaries are split so GUI code can render them differently if needed.
  */
struct DashboardSnapshot {
    std::vector<StatSummary> numericStats;
    std::vector<ActionSummary> actionStats;
};

 /**
  * @class StatsTracker
  * Tracks named statistics and prepares summaries for dashboard output.
  */
class StatsTracker {
public:
    /**
     * Default constructor for the StatsTracker
     */
    StatsTracker() = default;

    /*@brief Builds a full dashboard snapshot of all tracked data.
     * @param analytics Analytics manager that owns the underlying logs.
     * @return Combined snapshot of numeric series summaries and action summaries.
     */

    /**
     * Builds a full dashboard snapshot of all tracked data.
     * @param analytics Analytics manager that owns the underlying logs.
     * @return Combined snapshot of numeric series summaries and action summaries.
     */
    DashboardSnapshot BuildSnapshot(const AnalyticsManager& analytics) const;

    /**
     * Builds a summary for one numeric series.
     * @param key Stable identifier for the series.
     * @param label Human-readable name for dashboard display.
     * @param log Existing DataLog from the analytics layer.
     * @return Summary data if the series contains samples, otherwise std::nullopt.
     */
    std::optional<StatSummary> BuildSeriesSummary(const std::string& key, const std::string& label,
                                                  const DataLog& log) const;

    /**
     * Builds a summary for one action log.
     * @param key Stable identifier for the action summary.
     * @param label Human-readable name for dashboard display.
     * @param log Existing ActionLog from the analytics layer.
     * @return Summary data if the log contains actions, otherwise std::nullopt.
     */
    std::optional<ActionSummary> BuildActionSummary(const std::string& key, const std::string& label,
                                                    const ActionLog& log) const;

    /**
     * Prepares and outputs dashboard data through the analytics manager.
     * Used as a temporary way to output the summaries to the console until the GUI is implemented.
     * @param Analytics manager that owns the underlying logs.
     */
    void DisplayData(const AnalyticsManager& analytics) const; 
};

} // namespace cse498
