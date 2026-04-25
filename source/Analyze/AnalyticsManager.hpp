/**
 *
 * @note Status: PROPOSAL
 *
 * The goal of this class is to act as a container to store all the DataLogs and
 * ActionLogs for the project. This class will be called by the game to log data
 * and actions which will be used by the StatsTracker class.
 * Used AI to create function comments
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
    DataLog mEnemiesKilledLog;
    DataLog mDamageDealtLog;

    ActionLog mActionLog; // Stores the players actions over time

    RunStats mCurrentRunStats; // Struct to hold stats for the current run

public:
    /**
     * Default constructor for the AnalyticsManager
     */
    AnalyticsManager() = default;

    /**
     * Resets the current run stats to their default values.
     */
    void ResetCurrentRunStats() { mCurrentRunStats = {}; }

    /**
     * Logs the damage per run
     * @param amount damage
     */
    void LogRunDamage(double amount);

    /**
     * Logs the number of enemies killed per run
     * @param count enemies
     */
    void LogRunEnemiesKilled(int count);

    /**
     * Returns the current run stats as a struct.
     */
    RunStats GetCurrentRunStats() const { return mCurrentRunStats; };

    /*
    Resets the enemies killed log
    */
    void ResetEnemiesKilledLog();

    /**
     * Clears the damage dealt log
     */
    void ResetDamageDealtLog();

    /**
     * Clears the action log
     */
    void ResetActionLog();

    /**
     * Resets all logs stored in the AnalyticsManager
     */
    void Reset();
    
    /**
     * Adds a new enemy count value to the enemies killed log
     * @param count_enemies The number of enemies killed
     */
    void LogEnemiesKilled(double count_enemies);

    /**
     * Adds a new damage dealt value to the damage dealt log
     * @param damage The damage dealt value to be added to the log
     */
    void LogDamageDealt(double damage);

    /*
    Returns a const reference to the enemies killed log
    */
    [[nodiscard]] const DataLog& GetEnemiesKilledLog() const noexcept;

    /**
     * Returns a const reference to the damage dealt log
     * @return A const reference to the damage dealt log
     */
    [[nodiscard]] const DataLog& GetDamageDealtLog() const noexcept;

    /**
     * Logs an action performed by an entity into the player action log
     * @param entityId The ID of the entity that performed the action
     * @param actionType The type of action performed
     * @param position The position of the entity before the action
     * @param newPosition The position of the entity after the action
     */
    void LogAction(int entityId, const std::string& actionType, WorldPosition position, WorldPosition newPosition);

    /**
     * Advances the action log's simulation clock
     * @param newTime The new time value for the simulation clock
     */
    void UpdateActionTime(double newTime);

    /**
     * Returns a const reference to the action log
     * @return A const reference to the action log
     */
    [[nodiscard]] const ActionLog& GetActionLog() const noexcept;
};

} // namespace cse498
