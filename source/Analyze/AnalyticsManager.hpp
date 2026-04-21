/**
 *
 * @note Status: PROPOSAL
 *
 * The goal of this class is to act as a container to store all the DataLogs and
 * ActionLogs for the project. This class will be called by the game to log data
 * and actions which will be used by the StatsTracker class.
 **/

#pragma once
#include <string>
#include "../tools/ActionLog.hpp"
#include "../tools/DataLog.hpp"

namespace cse498 {

// Struct to hold stats for a single run, can be expanded as needed
struct RunStats {
    double damageDealt = 0.0;
    int enemiesKilled = 0;
};

class AnalyticsManager {
private:
    // Initial DataLogs for keeping track of data(Initial placeholders Logs for
    // now, can be changed based on what we want to track)
    DataLog mEnemiesKilledLog;
    DataLog mDamageDealtLog;


    // ActionLog for tracking entity actions
    ActionLog mActionLog;

    RunStats mCurrentRunStats; // Struct to hold stats for the current run

public:
    /*
    Default constructor for the AnalyticsManager
    */
    AnalyticsManager() = default;

    /**
     * Resets the current run stats to their default values.
     */
    void ResetCurrentRunStats() { mCurrentRunStats = {}; }

    void LogRunDamage(double amount);

    void LogRunEnemiesKilled(int count);

    /**
     * Returns the current run stats as a struct.
     */
    RunStats GetCurrentRunStats() const { return mCurrentRunStats; };

    /*
    Resets the enemies killed log
    */
    void ResetEnemiesKilledLog();

    /*
    Resets the damage dealt log
    */
    void ResetDamageDealtLog();

    /**
     * Resets the action log
     */
    void ResetActionLog();

    /**
     * Resets all logs
     */
    void Reset();

    /*
    Adds a new enemy count value to the enemies killed log
    */
    void LogEnemiesKilled(double count_enemies);

    /*
    Adds a new damage dealt value to the damage dealt log
    */
    void LogDamageDealt(double damage);

    /*
    Returns a const reference to the enemies killed log
    */
    [[nodiscard]] const DataLog& GetEnemiesKilledLog() const noexcept;

    /*
    Returns a const reference to the damage dealt log
    */
    [[nodiscard]] const DataLog& GetDamageDealtLog() const noexcept;

    /*
    Logs an action performed by an entity into the action log
    */
    void LogAction(int entityId, const std::string& actionType, WorldPosition position, WorldPosition newPosition);

    /*
    Advances the action log's simulation clock
    */
    void UpdateActionTime(double newTime);

    /*
    Returns a const reference to the action log
    */
    [[nodiscard]] const ActionLog& GetActionLog() const noexcept;
};

} // namespace cse498
