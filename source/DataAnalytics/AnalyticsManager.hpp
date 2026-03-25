#pragma once
#include "../tools/DataLog.hpp"
// #include "../tools/ActionLog.hpp"

namespace cse498
{

class AnalyticsManager{
    private:
        // Temporary Logs used for connecting with the starter code
        // DataLog mSuccessfulMovesLog;
        // DataLog mBlockedMovesLog;

        //Initial DataLogs for keeping track of data(Initial placeholders Logs for now, can be changed based on what we want to track)
        DataLog mHealthLog;
        DataLog mEnemiesTrackedLog;
        DataLog mDamageDealtLog;

        //Functionallity for storing ActionLog will be added below


    public:
        AnalyticsManager() = default;

        void Reset();

        void LogHealth(double health);
        void LogEnemiesTracked(double count_enemies);
        void LogDamageDealt(double damage);

        const DataLog& GetHealthLog() const;
        const DataLog& GetEnemiesTrackedLog() const;
        const DataLog& GetDamageDealtLog() const;

        // void LogSuccessfulMove(double value);
        // void LogBlockedMove(double value);

        // const DataLog& GetSuccessfulMovesLog() const;
        // const DataLog& GetBlockedMovesLog() const;

};

}