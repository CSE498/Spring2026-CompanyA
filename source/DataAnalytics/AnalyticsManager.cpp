/**
 *
 * @note Status: PROPOSAL
 *
 * The goal of this class is to act as a container to store all the DataLogs and ActionLogs for the project.
 * This class will be called by the game to log data and actions which will be used by the StatsTracker class.
 **/

#include "AnalyticsManager.hpp"

namespace cse498
{

    void AnalyticsManager::Reset()
    {
        mHealthLog.Clear();
        mEnemiesTrackedLog.Clear();
        mDamageDealtLog.Clear();
    }

    void AnalyticsManager::LogHealth(double health)
    {
        mHealthLog.Add(health);
    }

    void AnalyticsManager::LogEnemiesTracked(double count_enemies)
    {
        mEnemiesTrackedLog.Add(count_enemies);
    }

    void AnalyticsManager::LogDamageDealt(double damage)
    {
        mDamageDealtLog.Add(damage);
    }

    const DataLog& AnalyticsManager::GetHealthLog() const
    {
        return mHealthLog;
    }

    const DataLog& AnalyticsManager::GetEnemiesTrackedLog() const
    {
        return mEnemiesTrackedLog;
    }

    const DataLog& AnalyticsManager::GetDamageDealtLog() const
    {
        return mDamageDealtLog;
    }
}