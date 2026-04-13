#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/OutputManager.hpp"

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

/** @brief Returns true if haystack contains needle
 * @param haystack The string to search within
 * @param needle The string to search for
 */
static bool Contains(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
}

/** @brief Extracts the leading timestamp in milliseconds from a formatted log line.
 * @param line The formatted log line.
 * @return The parsed millisecond value.
 */
static long long ParseTimestampMs(const std::string& line)
{
    const std::size_t start = line.find('[');
    const std::size_t end = line.find("ms]");

    REQUIRE(start != std::string::npos);
    REQUIRE(end != std::string::npos);
    REQUIRE(end > start + 1);

    return std::stoll(line.substr(start + 1, end - start - 1));
}

/*@brief Captures anything written to std::cout during the lifetime of this object
 */
class CoutCapture
{
public:
    CoutCapture()
        : m_old(std::cout.rdbuf(m_capture.rdbuf()))
    {
    }

    ~CoutCapture()
    {
        std::cout.rdbuf(m_old);
    }

    std::string Str() const
    {
        return m_capture.str();
    }

private:
    std::ostringstream m_capture;
    std::streambuf* m_old;
};

/*Tests for GetMinLogLevel and SetMinLogLevel functionality of OutputManager
*/
TEST_CASE("OutputManager::GetMinLogLevel returns current min level", "[output][minlevel]")
{
    cse498::OutputManager om;

    om.SetMinLogLevel(cse498::LogLevel::Warn);
    CHECK(om.GetMinLogLevel() == cse498::LogLevel::Warn);

    om.SetMinLogLevel(cse498::LogLevel::Debug);
    CHECK(om.GetMinLogLevel() == cse498::LogLevel::Debug);
}

/* @brief Tests for OutputManager functionality: log level filtering
*/
TEST_CASE("OutputManager::ShouldLog respects minimum log level", "[output][shouldlog]")
{
    cse498::OutputManager om;

    SECTION("Min = DEBUG allows DEBUG..ERROR")
    {
        om.SetMinLogLevel(cse498::LogLevel::Debug);

        CHECK(om.ShouldLog(cse498::LogLevel::Debug));
        CHECK(om.ShouldLog(cse498::LogLevel::Verbose));
        CHECK(om.ShouldLog(cse498::LogLevel::Info));
        CHECK(om.ShouldLog(cse498::LogLevel::Warn));
        CHECK(om.ShouldLog(cse498::LogLevel::Error));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Silent));
    }

    SECTION("Min = Silent filters everything")
    {
        om.SetMinLogLevel(cse498::LogLevel::Silent);

        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Debug));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Verbose));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Info));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Warn));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Error));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Silent));
    }
}

/* @brief Tests for OutputManager::Log behavior with filtering and formatting
*/
TEST_CASE("OutputManager::Log is filtered when below minimum level", "[output][log][filter]")
{
    cse498::OutputManager om;

    CoutCapture cap;
    om.SetMinLogLevel(cse498::LogLevel::Warn);
    om.Log(cse498::LogLevel::Info, cse498::LogCategory::System, "This should NOT print");

    CHECK(cap.Str().empty());
}

TEST_CASE("OutputManager::Log formats level/category/message (timestamps disabled)", "[output][log][format]")
{
    cse498::OutputManager om;
    om.SetMinLogLevel(cse498::LogLevel::Info);
    om.EnableTimestamps(false);

    CoutCapture cap;
    om.Log(cse498::LogLevel::Info, cse498::LogCategory::Combat, "Goblin hit player");
    const std::string out = cap.Str();

    CHECK_FALSE(out.empty());
    CHECK(Contains(out, "[INFO]"));
    CHECK(Contains(out, "[COMBAT]"));
    CHECK(Contains(out, "Goblin hit player"));
    CHECK_FALSE(Contains(out, "ms]"));
}

/* @brief Tests for OutputManager::Log behavior with timestamp toggling and Timer integration
*/
TEST_CASE("OutputManager timestamp toggling affects output", "[output][log][timestamp]")
{
    cse498::OutputManager om;

    CoutCapture cap;
    om.SetMinLogLevel(cse498::LogLevel::Info);
    om.EnableTimestamps(true);
    om.Log(cse498::LogLevel::Info, cse498::LogCategory::System, "Startup");

    CHECK(Contains(cap.Str(), "ms]"));
}

TEST_CASE("OutputManager timestamps advance over time using the shared Timer", "[output][log][timer]")
{
    cse498::OutputManager om;
    std::vector<std::string> lines;

    om.ClearSinks();
    om.AddSink([&](std::string_view s) { lines.emplace_back(s); });
    om.SetMinLogLevel(cse498::LogLevel::Info);
    om.EnableTimestamps(true);

    om.Log(cse498::LogLevel::Info, cse498::LogCategory::System, "First");
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    om.Log(cse498::LogLevel::Info, cse498::LogCategory::System, "Second");

    REQUIRE(lines.size() == 2);
    CHECK(Contains(lines[0], "ms]"));
    CHECK(Contains(lines[1], "ms]"));

    const long long firstMs = ParseTimestampMs(lines[0]);
    const long long secondMs = ParseTimestampMs(lines[1]);

    CHECK(secondMs >= firstMs);
    CHECK(secondMs - firstMs >= 10);
}

TEST_CASE("OutputManager can reset the timestamp clock", "[output][log][reset-timestamp]")
{
    cse498::OutputManager om;
    std::vector<std::string> lines;

    om.ClearSinks();
    om.AddSink([&](std::string_view s) { lines.emplace_back(s); });
    om.SetMinLogLevel(cse498::LogLevel::Info);
    om.EnableTimestamps(true);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    om.Log(cse498::LogLevel::Info, cse498::LogCategory::System, "Before reset");
    const long long beforeResetMs = ParseTimestampMs(lines.back());

    om.ResetTimestampClock();
    om.Log(cse498::LogLevel::Info, cse498::LogCategory::System, "After reset");
    const long long afterResetMs = ParseTimestampMs(lines.back());

    CHECK(beforeResetMs >= 10);
    CHECK(afterResetMs <= 10);
}

TEST_CASE("OutputManager convenience logging uses default level and category", "[output][log][defaults]")
{
    cse498::OutputManager om;
    std::vector<std::string> lines;

    om.ClearSinks();
    om.AddSink([&](std::string_view s) { lines.emplace_back(s); });
    om.EnableTimestamps(false);
    om.SetMinLogLevel(cse498::LogLevel::Info);

    om.Log("Default message");

    REQUIRE(lines.size() == 1);
    CHECK(Contains(lines[0], "[INFO]"));
    CHECK(Contains(lines[0], "[GENERAL]"));
    CHECK(Contains(lines[0], "Default message"));
}

TEST_CASE("OutputManager supports runtime category strings", "[output][log][runtime-category]")
{
    cse498::OutputManager om;
    std::vector<std::string> lines;

    om.ClearSinks();
    om.AddSink([&](std::string_view s) { lines.emplace_back(s); });
    om.EnableTimestamps(false);
    om.SetMinLogLevel(cse498::LogLevel::Debug);

    om.Log(cse498::LogLevel::Verbose, "PHYSICS", "Applied force");

    REQUIRE(lines.size() == 1);
    CHECK(Contains(lines[0], "[VERBOSE]"));
    CHECK(Contains(lines[0], "[PHYSICS]"));
    CHECK(Contains(lines[0], "Applied force"));
}

/* @brief Tests for OutputManager sink behavior
*/
TEST_CASE("OutputManager sink behavior", "[output][sink]")
{
    cse498::OutputManager om;
    std::vector<std::string> lines;

    om.ClearSinks();
    om.AddSink([&](std::string_view s) { lines.emplace_back(s); });

    SECTION("Allowed log emits exactly one formatted line")
    {
        om.SetMinLogLevel(cse498::LogLevel::Info);
        om.EnableTimestamps(false);
        om.Log(cse498::LogLevel::Info, cse498::LogCategory::Combat, "Goblin hit player");

        REQUIRE(lines.size() == 1);
        CHECK(Contains(lines[0], "[INFO]"));
        CHECK(Contains(lines[0], "[COMBAT]"));
        CHECK(Contains(lines[0], "Goblin hit player"));
    }

    SECTION("Per-sink minimum level filters independently")
    {
        std::vector<std::string> warnOnlyLines;

        om.ClearSinks();
        om.AddSink([&](std::string_view s) { lines.emplace_back(s); }, cse498::LogLevel::Info);
        om.AddSink([&](std::string_view s) { warnOnlyLines.emplace_back(s); }, cse498::LogLevel::Warn);

        om.SetMinLogLevel(cse498::LogLevel::Debug);
        om.EnableTimestamps(false);

        om.Log(cse498::LogLevel::Info, cse498::LogCategory::System, "Info message");
        om.Log(cse498::LogLevel::Warn, cse498::LogCategory::System, "Warn message");

        REQUIRE(lines.size() == 2);
        REQUIRE(warnOnlyLines.size() == 1);
        CHECK(Contains(warnOnlyLines[0], "Warn message"));
    }
}

/* @brief Tests for OutputManager name helper functions
*/
TEST_CASE("OutputManager name helpers return correct strings", "[output][names]")
{
    CHECK(cse498::OutputManager::LevelName(cse498::LogLevel::Debug) == "DEBUG");
    CHECK(cse498::OutputManager::LevelName(cse498::LogLevel::Verbose) == "VERBOSE");
    CHECK(cse498::OutputManager::LevelName(cse498::LogLevel::Info) == "INFO");
    CHECK(cse498::OutputManager::LevelName(cse498::LogLevel::Warn) == "WARN");
    CHECK(cse498::OutputManager::LevelName(cse498::LogLevel::Error) == "ERROR");
    CHECK(cse498::OutputManager::LevelName(cse498::LogLevel::Silent) == "SILENT");

    CHECK(cse498::OutputManager::CategoryName(cse498::LogCategory::System) == "SYSTEM");
    CHECK(cse498::OutputManager::CategoryName(cse498::LogCategory::WorldGen) == "WORLDGEN");
    CHECK(cse498::OutputManager::CategoryName(cse498::LogCategory::Performance) == "PERF");
    CHECK(cse498::OutputManager::CategoryName(cse498::LogCategory::Network) == "NET");
}

/* @brief Tests for file path validation and state helpers
*/
TEST_CASE("OutputManager file path validation throws on invalid inputs", "[output][file][errors]")
{
    cse498::OutputManager om;

    REQUIRE_THROWS_AS(om.SetCsvPath("", true), std::invalid_argument);

    om.EnableCsv(true);
    REQUIRE_THROWS_AS(om.SetCsvPath(".", false), std::runtime_error);
}

TEST_CASE("OutputManager state helpers report current settings", "[output][state]")
{
    cse498::OutputManager om;

    CHECK_FALSE(om.TimestampsEnabled());
    CHECK_FALSE(om.CsvEnabled());

    om.EnableTimestamps(true);
    om.EnableCsv(true);

    CHECK(om.TimestampsEnabled());
    CHECK(om.CsvEnabled());

    om.EnableCsv(false);
    CHECK_FALSE(om.CsvEnabled());
}
