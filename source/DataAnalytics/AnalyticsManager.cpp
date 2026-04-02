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
void AnalyticsManager::ResetHealthLog() {
  mHealthLog.Clear();
}

/*
Resets the enemies tracked log
*/
void AnalyticsManager::ResetEnemiesTrackedLog() {
  mEnemiesTrackedLog.Clear();
}

/*
Resets the damage dealt log
*/
void AnalyticsManager::ResetDamageDealtLog() {
  mDamageDealtLog.Clear();
}

/**
 * Resets the action log
 */
void AnalyticsManager::ResetActionLog() {
  mActionLog.Clear();
}

/*
Adds a new health value to the health log
*/
void AnalyticsManager::LogHealth(double health) { mHealthLog.Add(health); }

/*
Adds a new enemy count value to the enemies tracked log
*/
void AnalyticsManager::LogEnemiesTracked(double count_enemies) {
  mEnemiesTrackedLog.Add(count_enemies);
}

/*
Adds a new damage dealt value to the damage dealt log
*/
void AnalyticsManager::LogDamageDealt(double damage) {
  mDamageDealtLog.Add(damage);
}

/*
Returns a const reference to the health log
*/
const DataLog &AnalyticsManager::GetHealthLog() const { return mHealthLog; }

/*
Returns a const reference to the enemies tracked log
*/
const DataLog &AnalyticsManager::GetEnemiesTrackedLog() const {
  return mEnemiesTrackedLog;
}

/*
Returns a const reference to the damage dealt log
*/
const DataLog &AnalyticsManager::GetDamageDealtLog() const {
  return mDamageDealtLog;
}

void AnalyticsManager::LogAction(int entityId, const std::string& actionType,
                                  WorldPosition position, WorldPosition newPosition) {
  mActionLog.LogAction(entityId, actionType, position, newPosition);
}

void AnalyticsManager::UpdateActionTime(double newTime) {
  mActionLog.UpdateTime(newTime);
}

const ActionLog& AnalyticsManager::GetActionLog() const {
  return mActionLog;
}

} // namespace cse498