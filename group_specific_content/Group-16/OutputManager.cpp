#include "OutputManager.hpp"
#include <iostream>
#include <string>

/*@brief Constructs an OutputManager and initializes the timestamp start point
 */
OutputManager::OutputManager() : m_startTime(std::chrono::steady_clock::now()) {
    // default sink that prints to std::cout
    m_sinks.emplace_back([](std::string_view line) { std::cout << line << "\n"; });
}

/*@brief Sets the minimum log level for output
 * @param level The minimum log level to set
 */
void OutputManager::SetMinLogLevel(LogLevel level) {
    std::scoped_lock lock(m_mutex); // Lock mutex
    m_min = level;
}

/*@brief Gets the minimum log level for output
 * @return The minimum log level
 */
LogLevel OutputManager::GetMinLogLevel() const {
    std::scoped_lock lock(m_mutex); // Lock mutex
    return m_min;
}

/*@brief Enables or disables timestamps in log output
 * @param enabled True to enable timestamps, false to disable
 */
void OutputManager::EnableTimestamps(bool enabled) {
    std::scoped_lock lock(m_mutex); // Lock mutex
    m_timestamps = enabled;
}

/*@brief Checks if a message at a given level should be logged
 * @param level The log level to check
 */
bool OutputManager::ShouldLog(LogLevel level) const {
    std::scoped_lock lock(m_mutex); // Lock mutex

    if (m_min == LogLevel::Silent) {
        return false;
    }
    // Compare log levels as uint8_t for correct ordering
    return static_cast<uint8_t>(level) >= static_cast<uint8_t>(m_min);
}

void OutputManager::Log(LogLevel level, LogCategory category, std::string_view message) {
    bool time_stamp_enabled = true;
    std::chrono::steady_clock::time_point start_copy;

    {
        std::scoped_lock lock(m_mutex);
        if (m_min == LogLevel::Silent || static_cast<uint8_t>(level) < static_cast<uint8_t>(m_min)) {
            return;
        }

        time_stamp_enabled = m_timestamps;
        start_copy = m_startTime;

        if (m_csvEnabled && m_csv.is_open()) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_copy).count();

            m_csv << elapsed << "," << LevelName(level) << "," << CategoryName(category) << ","
                  << "\"" << message << "\"\n";
            m_csv.flush(); // Ensure data is written immediately
        }
    }
    // Build the string for standard sinks (Console, etc.)
    std::string output;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_copy).count();

    if (time_stamp_enabled) {
        output += "[" + std::to_string(elapsed) + "ms] ";
    }

    output += "[" + std::string(LevelName(level)) + "] ";
    output += "[" + std::string(CategoryName(category)) + "] ";
    output += std::string(message);

    std::vector<LogSink> sinks_copy;
    {
        std::scoped_lock lock(m_mutex);
        sinks_copy = m_sinks;
    }

    for (auto& sink: sinks_copy) {
        sink(output);
    }
}
/*@brief returns uppercase string view of log level for output
 * @param category The log category to convert
 * @return A string view representing the log category
 */
std::string_view OutputManager::LevelName(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
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
    return "INFO"; // Default
}

/*@brief returns uppercase string view of log category for output
 * @param category The log category to convert
 * @return A string view representing the log category
 */
std::string_view OutputManager::CategoryName(LogCategory category) {
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
    return "SYSTEM"; // Default
}

/*@brief Adds a log sink to the output manager
 * @param sink The LogSink callback to add
 */
void OutputManager::AddSink(LogSink sink) {
    std::scoped_lock lock(m_mutex); // Lock mutex
    m_sinks.emplace_back(std::move(sink));
}

/*@brief Clears all registered log sinks
 * @details
 * After clearing sinks, Log() will not output anywhere until a sink is added.
 */
void OutputManager::ClearSinks() {
    std::scoped_lock lock(m_mutex); // Lock mutex
    m_sinks.clear();
}

/* @brief Enables or disables CSV logging
 * @param enabled True to enable CSV logging, false to disable
 */
void OutputManager::EnableCsv(bool enabled) {
    std::scoped_lock lock(m_mutex); // Lock mutex
    m_csvEnabled = enabled;
}

/*@brief Sets the CSV output file path
 * @param path File path to write CSV rows to
 * @param append True to append, false to overwrite
 */

void OutputManager::SetCsvPath(const std::string& path, bool append) {
    std::scoped_lock lock(m_mutex);
    m_csvPath = path;

    if (m_csv.is_open()) {
        m_csv.close();
    }

    // Set the file mode: append or truncate (overwrite)
    std::ios_base::openmode mode = std::ios::out;
    mode |= append ? std::ios::app : std::ios::trunc;

    m_csv.open(m_csvPath, mode);

    if (m_csv.is_open() && (!append || m_csv.tellp() == 0)) {
        m_csv << "Timestamp(ms),Level,Category,Message\n";
        m_csvHeaderWritten = true;
    }
}
