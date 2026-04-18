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

class AnalyticsManager {
private:
    /**
     * Stores the players health values over time
     */
    DataLog mHealthLog;

    /**
     * Stores the number of enemies over time
     */
    DataLog mEnemiesTrackedLog;

    /**
     * Stores the damage dealt by the player over time
     */
    DataLog mDamageDealtLog;


    /**
     * Stores the players actions over time
     */
    ActionLog mActionLog;

public:
    /**
     * Default constructor for the AnalyticsManager
     */
    AnalyticsManager() = default;

    /**
     * Clears the health log
     */
    void ResetHealthLog();

    /**
     * Clears the enemies tracked log
     */
    void ResetEnemiesTrackedLog();

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
     * Adds a new health value to the health log
     * @param health The health value to be added to the log
     */
    void LogHealth(double health);

    /**
     * Adds a new enemy count value to the enemies tracked log
     * @param count_enemies The enemy count value to be added to the log
     */
    void LogEnemiesTracked(double count_enemies);

    /**
     * Adds a new damage dealt value to the damage dealt log
     * @param damage The damage dealt value to be added to the log
     */
    void LogDamageDealt(double damage);

    /**
     * Returns a const reference to the health log
     * @return A const reference to the health log
     */
    [[nodiscard]] const DataLog& GetHealthLog() const noexcept;


    /**
     * Returns a const reference to the enemies tracked log
     * @return A const reference to the enemies tracked log
     */
    [[nodiscard]] const DataLog& GetEnemiesTrackedLog() const noexcept;

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
