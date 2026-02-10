#include "OutputManager.hpp"
#include <iostream>
#include <string>

/*@brief Constructs an OutputManager and initializes the timestamp start point
 */
OutputManager::OutputManager()
    : m_startTime(std::chrono::steady_clock::now())
{
}

/*@brief Sets the minimum log level for output
 * @param level The minimum log level to set
*/
void OutputManager::SetMinLogLevel(LogLevel level) 
{
    std::scoped_lock lock(m_mutex); // Lock mutex
    m_min = level;
}

/*@brief Gets the minimum log level for output
 * @return The minimum log level
*/
LogLevel OutputManager::GetMinLogLevel() const 
{
    std::scoped_lock lock(m_mutex); //Lock mutex
    return m_min;
}

/*@brief Enables or disables timestamps in log output
 * @param enabled True to enable timestamps, false to disable
*/
void OutputManager::EnableTimestamps(bool enabled) 
{
    std::scoped_lock lock(m_mutex); //Lock mutex
    m_timestamps = enabled;
}

/*@brief Checks if a message at a given level should be logged
 * @param level The log level to check
*/
bool OutputManager::ShouldLog(LogLevel level) const 
{
    std::scoped_lock lock(m_mutex); //Lock mutex

    if (m_min == LogLevel::Silent)
    {
        return false;
    }
    // Compare log levels as uint8_t for correct ordering
    return static_cast<uint8_t>(level) >= static_cast<uint8_t>(m_min);
}

void OutputManager::Log(LogLevel level, LogCategory category, std::string_view message) 
{   
    bool time_stamp_enabled = true;
    std::chrono::steady_clock::time_point start_copy;

    {
        std::scoped_lock lock(m_mutex); //Lock mutex

        if (m_min == LogLevel::Silent || static_cast<uint8_t>(level) < static_cast<uint8_t>(m_min)) 
        {
            return;
        }
        time_stamp_enabled = m_timestamps; //Copy timestamp flag under lock
        start_copy = m_startTime;
    }

    std::string output;

    if (time_stamp_enabled) 
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_copy).count();
        output += "[";
        output += std::to_string(elapsed);
        output += "ms] ";
    }

    output += "[";
    output += std::string(LevelName(level));
    output += "] ";

    output += "[";
    output += std::string(CategoryName(category));
    output += "] ";

    output += std::string(message);
    std::cout << output << "\n";

}
/*@brief returns uppercase string view of log level for output
 * @param category The log category to convert
 * @return A string view representing the log category
*/
std::string_view OutputManager::LevelName(LogLevel level) 
{
    switch (level) 
    {
        case LogLevel::DEBUG:   return "DEBUG";
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
std::string_view OutputManager::CategoryName(LogCategory category) 
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