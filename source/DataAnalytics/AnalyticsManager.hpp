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
        /*
        Default constructor for the AnalyticsManager
        */
        AnalyticsManager() = default;

        /*
        Resets all the logs
        */
        void Reset();

        //Temporary DataLogs for the game(can be changed based on what we want to track)
        /*
        Adds a new health value to the health log 
        */
        void LogHealth(double health);

        /*
        Adds a new enemy count value to the enemies tracked log
        */
        void LogEnemiesTracked(double count_enemies);

        /*
        Adds a new damage dealt value to the damage dealt log
        */
        void LogDamageDealt(double damage);

        /*
        Returns a const reference to the health log
        */
        const DataLog& GetHealthLog() const;

        /*
        Returns a const reference to the enemies tracked log
        */
        const DataLog& GetEnemiesTrackedLog() const;

        /*
        Returns a const reference to the damage dealt log
        */
        const DataLog& GetDamageDealtLog() const;

        //Functionality for ActionLog will be added below

};

}