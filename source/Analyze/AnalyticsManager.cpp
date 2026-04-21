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
 * Logs damage dealt in the current run and updates the total damage dealt.
 */
void AnalyticsManager::LogRunDamage(double amount) { mCurrentRunStats.damageDealt += amount; }

/**
 * Logs the number of enemies killed in the current run and updates the total enemies killed.
 */
void AnalyticsManager::LogRunEnemiesKilled(int count) { mCurrentRunStats.enemiesKilled += count; }

/*
Resets the enemies killed log
*/
void AnalyticsManager::ResetEnemiesKilledLog() { mEnemiesKilledLog.Clear(); }

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
    ResetEnemiesKilledLog();
    ResetDamageDealtLog();
    ResetActionLog();
}


/*
Adds a new enemy count value to the enemies killed log
*/
void AnalyticsManager::LogEnemiesKilled(double count_enemies) {
    // Ignore negative values
    if (count_enemies < 0) {
        return;
    }
    mEnemiesKilledLog.Add(count_enemies);
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

/*
Returns a const reference to the enemies killed log
*/
const DataLog& AnalyticsManager::GetEnemiesKilledLog() const noexcept { return mEnemiesKilledLog; }

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
