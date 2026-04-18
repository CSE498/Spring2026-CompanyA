/**
 *
 * @note Status: PROPOSAL
 *
 * The goal of this class is to act as a container to store all the DataLogs and
 * ActionLogs for the project. This class will be called by the game to log data
 * and actions which will be used by the StatsTracker class.
 * Used AI to help create function comments
 **/

#include "AnalyticsManager.hpp"

namespace cse498 {

/**
 * Clears the health log
 */
void AnalyticsManager::ResetHealthLog() { mHealthLog.Clear(); }

/**
 * Clears the enemies tracked log
 */
void AnalyticsManager::ResetEnemiesTrackedLog() { mEnemiesTrackedLog.Clear(); }

/**
 * Clears the damage dealt log
 */
void AnalyticsManager::ResetDamageDealtLog() { mDamageDealtLog.Clear(); }

/**
 * Resets the action log
 */
void AnalyticsManager::ResetActionLog() { mActionLog.Clear(); }

/**
 * Resets all logs
 */
void AnalyticsManager::Reset() {
    ResetHealthLog();
    ResetEnemiesTrackedLog();
    ResetDamageDealtLog();
    ResetActionLog();
}

/**
 * Adds a new health value to the health log
 * @param health The health value to be added to the log
 */
void AnalyticsManager::LogHealth(double health) {
    // Ignore negative values
    if (health < 0) {
        return;
    }
    mHealthLog.Add(health);
}

/**
 * Adds a new enemy count value to the enemies tracked log
 * @param count_enemies The enemy count value to be added to the log
 */
void AnalyticsManager::LogEnemiesTracked(double count_enemies) {
    // Ignore negative values
    if (count_enemies < 0) {
        return;
    }
    mEnemiesTrackedLog.Add(count_enemies);
}

/**
 * Adds a new damage dealt value to the damage dealt log
 * @param damage The damage dealt value to be added to the log
 */
void AnalyticsManager::LogDamageDealt(double damage) {
    // Ignore negative values
    if (damage < 0) {
        return;
    }
    mDamageDealtLog.Add(damage);
}

/**
 * Returns a const reference to the health log
 * @return A const reference to the health log
 */
const DataLog& AnalyticsManager::GetHealthLog() const noexcept { return mHealthLog; }

/**
 * Returns a const reference to the enemies tracked log
 * @return A const reference to the enemies tracked log
 */
const DataLog& AnalyticsManager::GetEnemiesTrackedLog() const noexcept { return mEnemiesTrackedLog; }

/**
 * Returns a const reference to the damage dealt log
 * @return A const reference to the damage dealt log
 */
const DataLog& AnalyticsManager::GetDamageDealtLog() const noexcept { return mDamageDealtLog; }

/**
 * Logs an action performed by an entity into the player action log
 * @param entityId The ID of the entity that performed the action
 * @param actionType The type of action performed
 * @param position The position of the entity before the action
 * @param newPosition The position of the entity after the action
 */
void AnalyticsManager::LogAction(int entityId, const std::string& actionType, WorldPosition position,
                                 WorldPosition newPosition) {
    mActionLog.LogAction(entityId, actionType, position, newPosition);
}

/**
 * Advances the action log's simulation clock
 * @param newTime The new time value for the simulation clock
 */
void AnalyticsManager::UpdateActionTime(double newTime) { mActionLog.UpdateTime(newTime); }

/**
 * Returns a const reference to the action log
 * @return A const reference to the action log
 */
const ActionLog& AnalyticsManager::GetActionLog() const noexcept { return mActionLog; }

} // namespace cse498
