#include "OutputManager.hpp"

#include <iostream>
#include <utility>

namespace {
    constexpr const char* DEFAULT_RUNTIME_CATEGORY = "GENERAL";
}

/*@brief Creates a new OutputManager.
 */
cse498::OutputManager::OutputManager() : m_timer("OutputManager") {
    // Default sink prints to the console so the logger works immediately.
    m_sinks.push_back({[](std::string_view line) { std::cout << line << '\n'; }, LogLevel::Debug});
}

/*@brief Sets the minimum log level for the manager.
 * @param level The minimum level that will be emitted.
 */
void cse498::OutputManager::SetMinLogLevel(LogLevel level) {
    std::scoped_lock lock(m_mutex);
    m_min = level;
}

/* @brief Gets the current minimum log level.
 * @return The active minimum log level.
 */
cse498::LogLevel cse498::OutputManager::GetMinLogLevel() const {
    std::scoped_lock lock(m_mutex);
    return m_min;
}

/* @brief Checks if a level would be emitted by the manager.
 * @param level The level to test.
 * @return True if the manager would allow the message through.
 */
bool cse498::OutputManager::ShouldLogUnlocked(LogLevel level) const {
    // Silent disables all output, both as a manager state and as a message level.
    if (level == LogLevel::Silent || m_min == LogLevel::Silent) {
        return false;
    }

    return static_cast<uint8_t>(level) >= static_cast<uint8_t>(m_min);
}

/* @brief Checks if a level would be emitted by the manager.
 * @param level The level to test.
 * @return True if the manager would allow the message through.
 */
bool cse498::OutputManager::ShouldLog(LogLevel level) const {
    std::scoped_lock lock(m_mutex);
    return ShouldLogUnlocked(level);
}

/* @brief Enables or disables timestamps in the formatted output.
 * @param enabled True to include elapsed milliseconds in each line.
 */
void cse498::OutputManager::EnableTimestamps(bool enabled) {
    std::scoped_lock lock(m_mutex);
    m_timestamps = enabled;
}

/* @brief Reports whether timestamps are currently enabled.
 * @return True when timestamps are enabled.
 */
bool cse498::OutputManager::TimestampsEnabled() const {
    std::scoped_lock lock(m_mutex);
    return m_timestamps;
}

/* @brief Resets the timer used for timestamped log output.
 * @details
 * After calling this, future timestamped log lines start again near 0 ms.
 */
void cse498::OutputManager::ResetTimestampClock() {
    std::scoped_lock lock(m_mutex);
    m_timer.restart();
}

/* @brief Gets the elapsed time in milliseconds since the last call to ResetTimestampClock().
 * @return The elapsed time in milliseconds since the last call to ResetTimestampClock().
 */
long long cse498::OutputManager::ElapsedMilliseconds() const {
    const double elapsedSeconds = m_timer.elapsed();
    return static_cast<long long>(elapsedSeconds * 1000.0);
}

/* @brief Creates a formatted log line.
 * @param level The level of the message.
 * @param category The category of the message.
 * @param message The message to log.
 * @param includeTimestamp True to include elapsed milliseconds in the formatted line.
 * @param elapsedMs The elapsed time in milliseconds since the last call to ResetTimestampClock().
 * @return The formatted log line.
 */
std::string cse498::OutputManager::FormatLine(LogLevel level, std::string_view category, std::string_view message,
                                              bool includeTimestamp, long long elapsedMs) const {
    std::string output;

    // Timestamps are optional so the logger can stay lightweight for normal output.
    if (includeTimestamp) {
        output += "[" + std::to_string(elapsedMs) + "ms] ";
    }

    output += "[";
    output += LevelName(level);
    output += "] [";
    output += category;
    output += "] ";
    output += message;

    return output;
}

/* @brief Logs a message using the built-in category enum.
 * @param level The log level for the message.
 * @param category The built-in category for the message.
 * @param message The text to log.
 */
void cse498::OutputManager::Log(LogLevel level, LogCategory category, std::string_view message) {
    Log(level, CategoryName(category), message);
}

/* @brief Logs a message using a runtime-provided category name.
 * @param level The log level for the message.
 * @param category The category name to show in the output.
 * @param message The text to log.
 */
void cse498::OutputManager::Log(LogLevel level, std::string_view category, std::string_view message) {
    std::vector<SinkEntry> sinksCopy;
    bool includeTimestamp = false;
    long long elapsedMs = 0;

    {
        std::scoped_lock lock(m_mutex);
        if (!ShouldLogUnlocked(level)) {
            return;
        }

        includeTimestamp = m_timestamps;
        elapsedMs = ElapsedMilliseconds();
        sinksCopy = m_sinks;
    }

    // Empty runtime categories fall back to a safe default label.
    if (category.empty()) {
        category = DEFAULT_RUNTIME_CATEGORY;
    }

    const std::string line = FormatLine(level, category, message, includeTimestamp, elapsedMs);

    {
        std::scoped_lock lock(m_mutex);
        // File output uses the same formatted line as every other sink.
        if (m_csvEnabled && m_csv.is_open()) {
            m_csv << line << '\n';
            m_csv.flush();
        }
    }

    for (const SinkEntry& entry: sinksCopy) {
        if (level == LogLevel::Silent) {
            continue;
        }

        if (static_cast<uint8_t>(level) < static_cast<uint8_t>(entry.minLevel)) {
            continue;
        }

        entry.sink(line);
    }
}

/* @brief Logs a message with convenient defaults.
 * @param message The text to log at Info level with the GENERAL category.
 */
void cse498::OutputManager::Log(std::string_view message) { Log(LogLevel::Info, DEFAULT_RUNTIME_CATEGORY, message); }

/* @brief Converts a log level to its display name.
 * @param level The level to convert.
 * @return Uppercase text for the level.
 */
std::string_view cse498::OutputManager::LevelName(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Verbose:
            return "VERBOSE";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Silent:
            return "SILENT";
    }

    return "INFO";
}

/* @brief Converts a built-in category to its display name.
 * @param category The category to convert.
 * @return Uppercase text for the category.
 */
std::string_view cse498::OutputManager::CategoryName(LogCategory category) {
    switch (category) {
        case LogCategory::System:
            return "SYSTEM";
        case LogCategory::WorldGen:
            return "WORLDGEN";
        case LogCategory::World:
            return "WORLD";
        case LogCategory::Combat:
            return "COMBAT";
        case LogCategory::AI:
            return "AI";
        case LogCategory::Items:
            return "ITEMS";
        case LogCategory::Trade:
            return "TRADE";
        case LogCategory::Puzzle:
            return "PUZZLE";
        case LogCategory::Replay:
            return "REPLAY";
        case LogCategory::Performance:
            return "PERF";
        case LogCategory::UI:
            return "UI";
        case LogCategory::Network:
            return "NET";
    }

    return DEFAULT_RUNTIME_CATEGORY;
}

/* @brief Adds a sink to the manager.
 * @param sink The sink to add.
 * @param minLevel The minimum log level for the sink.
 */
void cse498::OutputManager::AddSink(LogSink sink, LogLevel minLevel) {
    std::scoped_lock lock(m_mutex);
    m_sinks.push_back({std::move(sink), minLevel});
}

/* @brief Removes all registered sinks.
 * @details
 * After this, no console or custom sink output will occur until a sink is added.
 */
void cse498::OutputManager::ClearSinks() {
    std::scoped_lock lock(m_mutex);
    m_sinks.clear();
}

/* @brief Opens the log file for writing.
 * @details
 * This is only called when a log file is specified.
 */
void cse498::OutputManager::OpenCsvLocked() {
    if (m_csvPath.empty()) {
        throw std::invalid_argument("CSV path cannot be empty");
    }

    if (m_csv.is_open()) {
        return;
    }

    std::ios_base::openmode mode = std::ios::out;
    mode |= m_csvAppend ? std::ios::app : std::ios::trunc;
    m_csv.open(m_csvPath, mode);

    if (!m_csv.is_open()) {
        throw std::runtime_error("Failed to open log file: " + m_csvPath);
    }
}

/* @brief Closes the log file if it's open.
 * @details
 * This is called when disabling CSV logging or changing the log file path.
 */
void cse498::OutputManager::CloseCsvLocked() {
    if (m_csv.is_open()) {
        m_csv.close();
    }
}

/* @brief Enables or disables CSV logging.
 * @param enabled True to enable CSV logging.
 */
void cse498::OutputManager::EnableCsv(bool enabled) {
    std::scoped_lock lock(m_mutex);

    if (enabled == m_csvEnabled) {
        return;
    }

    m_csvEnabled = enabled;

    if (!m_csvEnabled) {
        CloseCsvLocked();
        return;
    }

    if (!m_csvPath.empty()) {
        OpenCsvLocked();
    }
}

/* @brief Reports whether CSV logging is currently enabled.
 * @return True when CSV logging is enabled.
 */
bool cse498::OutputManager::CsvEnabled() const {
    std::scoped_lock lock(m_mutex);
    return m_csvEnabled;
}

/* @brief Sets the file path used for CSV logging.
 * @param path The file path to write log lines to.
 * @param append True to append to the file, false to overwrite it.
 */
void cse498::OutputManager::SetCsvPath(const std::string& path, bool append) {
    std::scoped_lock lock(m_mutex);

    if (path.empty()) {
        throw std::invalid_argument("CSV path cannot be empty");
    }

    m_csvPath = path;
    m_csvAppend = append;

    CloseCsvLocked();

    if (m_csvEnabled) {
        OpenCsvLocked();
    }
}
