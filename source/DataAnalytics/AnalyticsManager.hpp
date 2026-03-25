/**
 *
 * @note Status: PROPOSAL
 *
 * The goal of this class is to act as a container to store all the DataLogs and ActionLogs for the project.
 * This class will be called by the game to log data and actions which will be used by the StatsTracker class.
 **/

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

        //Functionality for storing ActionLog will be added below


    public:
        AnalyticsManager() = default;

        void Reset();

        //Functions to log data into a specific DataLog
        void LogHealth(double health);
        void LogEnemiesTracked(double count_enemies);
        void LogDamageDealt(double damage);

        //Returns const reference of the logs to be used by StatsTracker
        const DataLog& GetHealthLog() const;
        const DataLog& GetEnemiesTrackedLog() const;
        const DataLog& GetDamageDealtLog() const;

        //Functionality for ActionLog will be added below

};

}