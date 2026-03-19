#include "AnalyticsManager.hpp"

namespace cse498
{

    void AnalyticsManager::Reset()
    {
        mHealthLog.Clear();
        mEnemiesTrackedLog.Clear();
    }

    void AnalyticsManager::LogHealth(double health)
    {
        mHealthLog.Add(health);
    }

    void AnalyticsManager::LogEnemiesTracked(int count_enemies)
    {
        mEnemiesTrackedLog.Add(count_enemies);
    }

    const DataLog& AnalyticsManager::GetHealthLog() const
    {
        return mHealthLog;
    }

    const DataLog& AnalyticsManager::GetEnemiesTrackedLog() const
    {
        return mEnemiesTrackedLog;
    }

}