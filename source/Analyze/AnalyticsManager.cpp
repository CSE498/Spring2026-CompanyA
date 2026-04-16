/**
 *
 * @note Status: PROPOSAL
 *
 * The goal of this class is to act as a container to store all the DataLogs and
 * ActionLogs for the project. This class will be called by the game to log data
 * and actions which will be used by the StatsTracker class.
 **/

#include "AnalyticsManager.hpp"

namespace cse498 {

/*
Resets the health log
*/
void AnalyticsManager::ResetHealthLog() { mHealthLog.Clear(); }

/*
Resets the enemies tracked log
*/
void AnalyticsManager::ResetEnemiesTrackedLog() { mEnemiesTrackedLog.Clear(); }

/*
Resets the damage dealt log
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

/*
Adds a new health value to the health log
*/
void AnalyticsManager::LogHealth(double health) {
    // Ignore negative values
    if (health < 0) {
        return;
    }
    mHealthLog.Add(health);
}

/*
Adds a new enemy count value to the enemies tracked log
*/
void AnalyticsManager::LogEnemiesTracked(double count_enemies) {
    // Ignore negative values
    if (count_enemies < 0) {
        return;
    }
    mEnemiesTrackedLog.Add(count_enemies);
}

/*
Adds a new damage dealt value to the damage dealt log
*/
void AnalyticsManager::LogDamageDealt(double damage) {
    // Ignore negative values
    if (damage < 0) {
        return;
    }
    mDamageDealtLog.Add(damage);
}

/*
Returns a const reference to the health log
*/
const DataLog& AnalyticsManager::GetHealthLog() const noexcept { return mHealthLog; }

/*
Returns a const reference to the enemies tracked log
*/
const DataLog& AnalyticsManager::GetEnemiesTrackedLog() const noexcept { return mEnemiesTrackedLog; }

/*
Returns a const reference to the damage dealt log
*/
const DataLog& AnalyticsManager::GetDamageDealtLog() const noexcept { return mDamageDealtLog; }

/*
Logs an action performed by an entity into the action log
*/
void AnalyticsManager::LogAction(int entityId, const std::string& actionType, WorldPosition position,
                                 WorldPosition newPosition) {
    mActionLog.LogAction(entityId, actionType, position, newPosition);
}

/*
Advances the action log's simulation clock
*/
void AnalyticsManager::UpdateActionTime(double newTime) { mActionLog.UpdateTime(newTime); }

/*
Returns a const reference to the action log
*/
const ActionLog& AnalyticsManager::GetActionLog() const noexcept { return mActionLog; }

} // namespace cse498
