/**
 * @file OutputManager.hpp
 * @author Tyler Murray
 * @brief Simple logging system for debugging and status output.
 *
 * OutputManager provides level-based logging, optional timestamps, configurable
 * sinks, and optional file output. It supports a built-in enum of common game
 * categories while also allowing callers to supply category names at runtime.
 */

#pragma once

#include "Timer.hpp"

#include <cstdint>
#include <fstream>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace cse498 {
    /*@enum LogLevel
     * @brief Different levels of logging severity.
     */
    enum class LogLevel : uint8_t { Debug = 0, Verbose = 1, Info = 2, Warn = 3, Error = 4, Silent = 5 };

    /*@enum LogCategory
     * @brief Common built-in categories for game logging.
     * @details
     * Callers may also provide categories at runtime through the string overload of Log().
     */
    enum class LogCategory : uint8_t {
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

    /*@brief Type alias for output callbacks.
     * @details
     * A LogSink receives one already-formatted log line.
     */
    using LogSink = std::function<void(std::string_view)>;

    /*@class OutputManager
     * @brief Simple logging system for debugging and status output.
     */
    class OutputManager {
    public:
        OutputManager();
        ~OutputManager() = default;

        /*@brief Sets the minimum log level for the manager.
         * @param level The minimum level that will be emitted.
         */
        void SetMinLogLevel(LogLevel level);

        /*@brief Gets the current minimum log level.
         * @return The active minimum log level.
         */
        LogLevel GetMinLogLevel() const;

        /*@brief Checks if a level would be emitted by the manager.
         * @param level The level to test.
         * @return True if the manager would allow the message through.
         */
        bool ShouldLog(LogLevel level) const;

        /*@brief Enables or disables timestamps in the formatted output.
         * @param enabled True to include elapsed milliseconds in each line.
         */
        void EnableTimestamps(bool enabled);

        /*@brief Reports whether timestamps are currently enabled.
         * @return True when timestamps are enabled.
         */
        bool TimestampsEnabled() const;

        /*@brief Resets the timer used for timestamped log output.
         * @details
         * After calling this, future timestamped log lines start again near 0 ms.
         */
        void ResetTimestampClock();

        /*@brief Logs a message using the built-in category enum.
         * @param level The log level for the message.
         * @param category The built-in category for the message.
         * @param message The text to log.
         */
        void Log(LogLevel level = LogLevel::Info, LogCategory category = LogCategory::System,
                 std::string_view message = "");

        /*@brief Logs a message using a runtime-provided category name.
         * @param level The log level for the message.
         * @param category The category name to show in the output.
         * @param message The text to log.
         */
        void Log(LogLevel level, std::string_view category, std::string_view message);

        /*@brief Logs a message with convenient defaults.
         * @param message The text to log at Info level with the GENERAL category.
         */
        void Log(std::string_view message);

        /*@brief Converts a log level to its display name.
         * @param level The level to convert.
         * @return Uppercase text for the level.
         */
        static std::string_view LevelName(LogLevel level);

        /*@brief Converts a built-in category to its display name.
         * @param category The category to convert.
         * @return Uppercase text for the category.
         */
        static std::string_view CategoryName(LogCategory category);

        /*@brief Adds a sink that receives formatted log lines.
         * @param sink The callback to invoke for each accepted line.
         * @param minLevel The minimum level this sink will receive.
         */
        void AddSink(LogSink sink, LogLevel minLevel = LogLevel::Debug);

        /*@brief Removes all registered sinks.
         * @details
         * After this, no console or custom sink output will occur until a sink is added.
         */
        void ClearSinks();

        /*@brief Enables or disables file logging.
         * @param enabled True to write formatted lines to the configured file path.
         */
        void EnableCsv(bool enabled);

        /*@brief Reports whether file logging is currently enabled.
         * @return True when file logging is enabled.
         */
        bool CsvEnabled() const;

        /*@brief Sets the file path used for file logging.
         * @param path File path to write log lines to.
         * @param append True to append, false to overwrite.
         */
        void SetCsvPath(const std::string& path, bool append = true);

    private:
        // Each sink carries its own minimum level so sinks can filter independently.
        struct SinkEntry {
            LogSink sink;
            LogLevel minLevel{LogLevel::Debug};
        };

        mutable std::mutex m_mutex;
        LogLevel m_min{LogLevel::Info};
        bool m_timestamps{false};
        Timer m_timer;
        std::vector<SinkEntry> m_sinks;

        bool m_csvEnabled{false};
        std::string m_csvPath;
        std::ofstream m_csv;
        bool m_csvAppend{true};

        // Internal helper that assumes the caller already holds the mutex.
        bool ShouldLogUnlocked(LogLevel level) const;

        // Elapsed time is measured by the shared Timer class relative to construction.
        long long ElapsedMilliseconds() const;

        // Builds the single standard line format used by all sinks and file output.
        std::string FormatLine(LogLevel level, std::string_view category, std::string_view message,
                               bool includeTimestamp, long long elapsedMs) const;

        void OpenCsvLocked();
        void CloseCsvLocked();
    };
} // namespace cse498
