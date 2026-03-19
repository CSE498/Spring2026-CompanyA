#pragma once
#include "DataLog.hpp"
#include "ActionLog.hpp"

namespace cse498
{

class AnalyticsManager{
    private:
        DataLog mHealthLog;
        DataLog mEnemiesTrackedLog;


    public:
        AnalyticsManager() = default;

        void Reset();

        void LogHealth(double health);
        void LogEnemiesTracked(int num_enemies);

        const DataLog& GetHealthLog() const;
        const DataLog& GetEnemiesTrackedLog() const;

};

}