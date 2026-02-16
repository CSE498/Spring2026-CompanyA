/**
 * * @file OutputManager.hpp
 * * @author Tyler Murray
 * * @brief Logging system for game engine
 * * @version 1.0
 * 
 * OutputManager is a category based logging system used across each module. It will
 * support logging levels, timestamps, and pluggable output sings. It will also log things
 * to a csv file.
 */

#pragma once
#include <cstdint>
#include <string>
#include <chrono>
#include <string_view>
#include <mutex>
#include <functional>
#include <vector>
#include <functional>
#include <fstream>


/*@enum LogLevel
 * @brief Different levels of logging severity
 * @details
 * DEBUG: Detailed information, typically of interest only when diagnosing problems.
 * Verbose: More detailed than Info, but less than Debug.
 * Info: Confirmation that things are working as expected.
 * Warn: An indication that something unexpected happened, or indicative of some problem in the near future.
 * Error: Due to a more serious problem, the software has not been able to perform some function.
 * Silent: No logging output.
 */

enum class LogLevel : uint8_t 
{
    DEBUG = 0,
    Verbose = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Silent = 5
};

/*
*@enum LogCategory
*@brief Different categories for loggigng so teams can filter or search output easily
*@details
* System: Logs related to system operations and events
* WorldGen: Logs related to world generation
* World: Logs related to world state and changes
* Combat: Logs related to combat mechanics and events
* AI: Logs related to AI behavior and events
* Items: Logs related to item usage and events
* Trade: Logs related to trading and events
* Puzzle: Logs related to puzzle solving and events
* Replay: Logs related to replay and events
* Performance: Logs related to performance and events
* UI: Logs related to UI and events
* Network: Logs related to network and events
*/
enum class LogCategory : uint8_t
{
    System,
    WorldGen,
    World,
    Combat,
    AI,
    Items,
    Trade,
    Puzzle,
    Replay,
    Performance,
    UI,
    Network
};

using LogSink = std::function<void(std::string_view)>;

/*@class OutputManager
 * @brief Logging system for game engine
*/
namespace cse498
{
    class OutputManager 
    {
    public:
        OutputManager();
        ~OutputManager() = default;
        
        /*@brief Sets the minimum log level for output
        * @param level The minimum log level to set
        */
        void SetMinLogLevel(LogLevel level);

        /*@brief Gets the minimum log level for output
        * @return The minimum log level
        */
        LogLevel GetMinLogLevel() const;

        /*@brief Checks if a message at a given level should be logged
        * @param level The log level to check
        */
        bool ShouldLog(LogLevel level) const;

        /*@brief Enables or disables timestamps in log output
        * @param enabled True to enable timestamps, false to disable
        */
        void EnableTimestamps(bool enabled);

        /*@brief Logs a message at a given level and category
        * @param level The log level of the message
        * @param category The log category of the message
        * @param message The message to log
        */
        void Log(LogLevel level, LogCategory category, std::string_view message);

        /*@brief returns uppercase string view of log level for output
        * @param level The log level to convert
        * @return A string view representing the log level
        */
        static std::string_view LevelName(LogLevel level);

        /*@brief returns uppercase string view of log category for output
        * @param category The log category to convert
        * @return A string view representing the log category
        */
        static std::string_view CategoryName(LogCategory category);

        /*@brief Adds a sink that receives each formatted log line
        * @param sink The sink callback to add
        */
        void AddSink(LogSink sink);

        /*@brief Clears all registered sinks
        * @details
        * After clearing sinks, Log() will not output anywhere until a sink is added.
        */
        void ClearSinks();

        /*@brief Enables or disables CSV logging
        * @param enabled True to enable CSV logging, false to disable
        */
        void EnableCsv(bool enabled);

        /*@brief Sets the CSV output file path
        * @param path File path to write CSV rows to
        * @param append True to append, false to overwrite
        */
        void SetCsvPath(const std::string& path, bool append = true);

    private:
        mutable std::mutex m_mutex; /// Mutex for thread operations
        LogLevel m_min{LogLevel::Info}; /// Minimum log level

        bool m_timestamps{false}; /// Flag for timestamps
        std::chrono::steady_clock::time_point m_startTime; /// Start time
        std::vector<LogSink> m_sinks; /// Registered output sinks

        bool m_csvEnabled{false};          /// CSV enabled flag
        std::string m_csvPath;             /// CSV output file path
        std::ofstream m_csv;               /// CSV stream
        bool m_csvHeaderWritten{false};    /// Header written flag
    };
}