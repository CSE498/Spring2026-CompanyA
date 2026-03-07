#include "OutputManager.hpp"
#include <iostream>
#include <string>


/*@brief Constructs an OutputManager and initializes the timestamp start point
 */
cse498::OutputManager::OutputManager()
    : m_startTime(std::chrono::steady_clock::now())
{
    //default sink that prints to std::cout
    m_sinks.emplace_back([](std::string_view line) {
        std::cout << line << "\n";
    });
}

/*@brief Sets the minimum log level for output
 * @param level The minimum log level to set
*/
void cse498::OutputManager::SetMinLogLevel(LogLevel level) 
{
    std::scoped_lock lock(m_mutex);
    m_min = level;
}

/*@brief Gets the minimum log level for output
 * @return The minimum log level
*/
cse498::LogLevel cse498::OutputManager::GetMinLogLevel() const 
{
    std::scoped_lock lock(m_mutex);
    return m_min;
}

/*@brief Enables or disables timestamps in log output
 * @param enabled True to enable timestamps, false to disable
*/
void cse498::OutputManager::EnableTimestamps(bool enabled) 
{
    std::scoped_lock lock(m_mutex);
    m_timestamps = enabled;
}

/*@brief Checks if a message at a given level should be logged
 * @param level The log level to check
*/
bool cse498::OutputManager::ShouldLog(LogLevel level) const 
{
    std::scoped_lock lock(m_mutex);

    if (m_min == LogLevel::Silent)
    {
        return false;
    }
    // Compare log levels as uint8_t for correct ordering
    return static_cast<uint8_t>(level) >= static_cast<uint8_t>(m_min);
}
/*@brief Logs a message with the specified level and category
 * @param level The log level of the message
 * @param category The log category of the message
 * @param message The message to log
*/
void cse498::OutputManager::Log(LogLevel level, LogCategory category, std::string_view message) 
{   
    bool time_stamp_enabled = true;
    std::chrono::steady_clock::time_point start_copy;

    {
        std::scoped_lock lock(m_mutex);
        if (m_min == LogLevel::Silent || static_cast<uint8_t>(level) < static_cast<uint8_t>(m_min)) 
        {
            return;
        }

        time_stamp_enabled = m_timestamps;
        start_copy = m_startTime;

        if (m_csvEnabled && m_csv.is_open()) 
        {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_copy).count();
            
            m_csv << elapsed << ","
                  << LevelName(level) << ","
                  << CategoryName(category) << ","
                  << "\"" << message << "\"\n";
            m_csv.flush(); // Ensure data is written immediately
        }
    }
// Build the string for standard sinks (Console, etc.)
    std::string output;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_copy).count(); // Calculate elapsed time for timestamp

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

    for (auto& sink : sinks_copy) {
        sink(output);
    }
}
/*@brief returns uppercase string view of log level for output
 * @param category The log category to convert
 * @return A string view representing the log category //test modify
*/
std::string_view cse498::OutputManager::LevelName(LogLevel level) const 
{
    switch (level)
    {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Verbose: return "VERBOSE";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warn:    return "WARN";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Silent:  return "SILENT";
    }
    return "INFO"; //Default
}

/*@brief returns uppercase string view of log category for output
 * @param category The log category to convert
 * @return A string view representing the log category
*/
std::string_view cse498::OutputManager::CategoryName(LogCategory category) const
{
    switch (category) 
    {
        case LogCategory::System:   return "SYSTEM";
        case LogCategory::WorldGen: return "WORLDGEN";
        case LogCategory::World:    return "WORLD";
        case LogCategory::Combat:   return "COMBAT";
        case LogCategory::AI:       return "AI";
        case LogCategory::Items:    return "ITEMS";
        case LogCategory::Trade:    return "TRADE";
        case LogCategory::Puzzle:   return "PUZZLE";
        case LogCategory::Replay:   return "REPLAY";
        case LogCategory::Performance:     return "PERF";
        case LogCategory::UI:       return "UI";
        case LogCategory::Network:  return "NET";
    }
    return "SYSTEM"; //Default
}

/*@brief Adds a log sink to the output manager
 * @param sink The LogSink callback to add
*/
void cse498::OutputManager::AddSink(LogSink sink)
{
    std::scoped_lock lock(m_mutex); 
    m_sinks.emplace_back(std::move(sink));
}

/*@brief Clears all registered log sinks
 * @details
 * After clearing sinks, Log() will not output anywhere until a sink is added.
*/
void cse498::OutputManager::ClearSinks()
{
    std::scoped_lock lock(m_mutex); 
    m_sinks.clear();
}

/*@brief Opens the CSV file for logging, writing the header if necessary
 * @details This should only be called while holding m_mutex.
*/
void cse498::OutputManager::OpenCsvLocked()
{
    if (m_csvPath.empty())
    {
        throw std::invalid_argument("CSV path is empty");
    }

    if (m_csv.is_open())
    {
        return; // already open
    }

    std::ios_base::openmode mode = std::ios::out;
    mode |= m_csvAppend ? std::ios::app : std::ios::trunc;

    m_csv.open(m_csvPath, mode);

    if (!m_csv.is_open())
    {
        throw std::runtime_error("Failed to open CSV file: " + m_csvPath);
    }

    // Write header if overwriting OR file is empty
    if (!m_csvAppend || m_csv.tellp() == 0)
    {
        m_csv << "Timestamp(ms),Level,Category,Message\n";
        m_csvHeaderWritten = true;
    }
}

/*@brief Closes the CSV file if it's open
 * @details
 * This should only be called while holding m_mutex.
*/
void cse498::OutputManager::CloseCsvLocked()
{
    if (m_csv.is_open())
    {
        m_csv.close();
    }
}


/* @brief Enables or disables CSV logging
 * @param enabled True to enable CSV logging, false to disable
*/
void cse498::OutputManager::EnableCsv(bool enabled)
{
    std::scoped_lock lock(m_mutex);

    if (enabled == m_csvEnabled)
    {
        return;
    }

    m_csvEnabled = enabled;

    if (!m_csvEnabled)
    {
        CloseCsvLocked();
        return;
    }

    // enabling
    if (!m_csvPath.empty())
    {
        OpenCsvLocked();
    }
}

/*@brief Sets the CSV output file path
 * @param path File path to write CSV rows to
 * @param append True to append, false to overwrite
*/
void cse498::OutputManager::SetCsvPath(const std::string& path, bool append)
{
    std::scoped_lock lock(m_mutex);

    if (path.empty())
    {
        throw std::invalid_argument("CSV path cannot be empty");
    }

    // store config
    m_csvPath = path;
    m_csvAppend = append;

    // if there was an open CSV, close it because we're switching targets
    CloseCsvLocked();

    // if CSV logging is currently enabled, open immediately so errors surface now
    if (m_csvEnabled)
    {
        OpenCsvLocked();
    }
}