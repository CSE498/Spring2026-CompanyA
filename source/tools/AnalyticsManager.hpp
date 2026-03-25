#pragma once
#include "DataLog.hpp"
#include "ActionLog.hpp"

namespace cse498
{

class AnalyticsManager{
    private:
        DataLog mSuccessfulMovesLog;
        DataLog mBlockedMovesLog;


        DataLog mHealthLog;
        DataLog mEnemiesTrackedLog;
        DataLog mDamageDealtLog;


    public:
        AnalyticsManager() = default;

        void LogSuccessfulMove(double value);
        void LogBlockedMove(double value);

        const DataLog& GetSuccessfulMovesLog() const;
        const DataLog& GetBlockedMovesLog() const;

        void Reset();

        void LogHealth(double health);
        void LogEnemiesTracked(double count_enemies);
        void LogDamageDealt(double damage);

        
        const DataLog& GetHealthLog() const;
        const DataLog& GetEnemiesTrackedLog() const;
        const DataLog& GetDamageDealtLog() const;

};

}