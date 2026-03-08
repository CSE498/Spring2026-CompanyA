#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/OutputManager.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>

/** @brief Returns true if haystack contains needle
 * @param haystack The string to search within
 * @param needle The string to search for
 */
static bool Contains(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
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
    }

    SECTION("Min = Verbose filters DEBUG, allows Verbose..ERROR")
    {
        om.SetMinLogLevel(cse498::LogLevel::Verbose);

        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Debug));
        CHECK(om.ShouldLog(cse498::LogLevel::Verbose));
        CHECK(om.ShouldLog(cse498::LogLevel::Info));
        CHECK(om.ShouldLog(cse498::LogLevel::Warn));
        CHECK(om.ShouldLog(cse498::LogLevel::Error));
    }

    SECTION("Min = Info filters DEBUG/Verbose, allows Info..Error")
    {
        om.SetMinLogLevel(cse498::LogLevel::Info);

        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Debug));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Verbose));
        CHECK(om.ShouldLog(cse498::LogLevel::Info));
        CHECK(om.ShouldLog(cse498::LogLevel::Warn));
        CHECK(om.ShouldLog(cse498::LogLevel::Error));
    }

    SECTION("Min = Warn filters <Warn, allows Warn..Error")
    {
        om.SetMinLogLevel(cse498::LogLevel::Warn);

        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Debug));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Verbose));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Info));
        CHECK(om.ShouldLog(cse498::LogLevel::Warn));
        CHECK(om.ShouldLog(cse498::LogLevel::Error));
    }

    SECTION("Min = Error filters <Error, allows Error only")
    {
        om.SetMinLogLevel(cse498::LogLevel::Error);

        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Debug));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Verbose));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Info));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Warn));
        CHECK(om.ShouldLog(cse498::LogLevel::Error));
    }

    SECTION("Min = Silent filters everything (including DEBUG..ERROR)")
    {
        om.SetMinLogLevel(cse498::LogLevel::Silent);

        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Debug));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Verbose));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Info));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Warn));
        CHECK_FALSE(om.ShouldLog(cse498::LogLevel::Error));
    }
}

/* @brief Tests for OutputManager::Log behavior with category filtering, formatting, and timestamp toggling
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

    SECTION("Combat category")
    {
        CoutCapture cap;
        om.Log(cse498::LogLevel::Info, cse498::LogCategory::Combat, "Goblin hit player");
        const std::string out = cap.Str();

        CHECK_FALSE(out.empty());
        CHECK(Contains(out, "[INFO]"));
        CHECK(Contains(out, "[COMBAT]"));
        CHECK(Contains(out, "Goblin hit player"));
        CHECK_FALSE(Contains(out, "ms]"));
    }

    SECTION("Puzzle category")
    {
        CoutCapture cap;
        om.Log(cse498::LogLevel::Info, cse498::LogCategory::Puzzle, "Player solved riddle");
        const std::string out = cap.Str();

        CHECK_FALSE(out.empty());
        CHECK(Contains(out, "[INFO]"));
        CHECK(Contains(out, "[PUZZLE]"));
        CHECK(Contains(out, "Player solved riddle"));
        CHECK_FALSE(Contains(out, "ms]"));
    }
}

/* @brief Tests for OutputManager::Log behavior with timestamp toggling
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
/* @brief Tests for OutputManager sink behavior, including filtering, formatting, and timestamp toggling
*/
TEST_CASE("OutputManager sink behavior (Step 3)", "[output][sink]")
{
    cse498::OutputManager om;
    std::vector<std::string> lines;

    // Force logs to the vector for testing by clearing default sinks and adding a custom one
    om.ClearSinks();
    om.AddSink([&](std::string_view s) { lines.emplace_back(s); });

    SECTION("Filtered logs emit nothing")
    {
        lines.clear();
        om.SetMinLogLevel(cse498::LogLevel::Warn);
        om.EnableTimestamps(false);
        om.Log(cse498::LogLevel::Info, cse498::LogCategory::System, "Should not appear");

        CHECK(lines.empty());
    }

    SECTION("Allowed log emits exactly one formatted line")
    {
        lines.clear();
        om.SetMinLogLevel(cse498::LogLevel::Info);
        om.EnableTimestamps(false);
        om.Log(cse498::LogLevel::Info, cse498::LogCategory::Combat, "Goblin hit player");

        CHECK(lines.size() == 1);
        CHECK(Contains(lines[0], "[INFO]"));
        CHECK(Contains(lines[0], "[COMBAT]"));
        CHECK(Contains(lines[0], "Goblin hit player"));
        CHECK_FALSE(Contains(lines[0], "ms]"));
    }

    SECTION("Timestamp toggle affects sink output")
    {
        lines.clear();
        om.SetMinLogLevel(cse498::LogLevel::Info);
        om.EnableTimestamps(true);
        om.Log(cse498::LogLevel::Info, cse498::LogCategory::System, "Startup");

        CHECK(lines.size() == 1);
        CHECK(Contains(lines[0], "ms]"));
    }
}
/* @brief Tests for OutputManager visual timestamp demo*/
// TEST_CASE("Visual timestamp demo (prints to console)", "[output][manual][visual]")
// {
//     cse498::OutputManager om;

//     std::cout << "\n=== Visual Timestamp Demo (should show ms) ===\n";

//     // Force logs to the console for visual inspection
//     om.ClearSinks();
//     om.AddSink([](std::string_view s) {
//         std::cout << s << "\n";
//     });

//     om.SetMinLogLevel(cse498::LogLevel::Info);
//     om.EnableTimestamps(true);

//     om.Log(cse498::LogLevel::Info, cse498::LogCategory::System, "Visual test: first message");
//     std::this_thread::sleep_for(std::chrono::milliseconds(25));
//     om.Log(cse498::LogLevel::Info, cse498::LogCategory::System, "Visual test: second message (later)");

//     SUCCEED(); // Test passed
// }

/* @brief Tests for OutputManager name helper functions
*/
TEST_CASE("OutputManager name helpers return correct strings", "[output][names]")
{
    cse498::OutputManager om;

    CHECK(om.LevelName(cse498::LogLevel::Debug) == "DEBUG");
    CHECK(om.LevelName(cse498::LogLevel::Verbose) == "VERBOSE");
    CHECK(om.LevelName(cse498::LogLevel::Info) == "INFO");
    CHECK(om.LevelName(cse498::LogLevel::Warn) == "WARN");
    CHECK(om.LevelName(cse498::LogLevel::Error) == "ERROR");
    CHECK(om.LevelName(cse498::LogLevel::Silent) == "SILENT");

    CHECK(om.CategoryName(cse498::LogCategory::System) == "SYSTEM");
    CHECK(om.CategoryName(cse498::LogCategory::WorldGen) == "WORLDGEN");
    CHECK(om.CategoryName(cse498::LogCategory::Performance) == "PERF");
    CHECK(om.CategoryName(cse498::LogCategory::Network) == "NET");
}

/* @brief Tests for OutputManager CSV path validation and error handling
*/
TEST_CASE("CSV path validation throws on invalid inputs", "[output][csv][errors]")
{
    cse498::OutputManager om;

    // empty path should always throw
    REQUIRE_THROWS_AS(om.SetCsvPath("", true), std::invalid_argument);

    // directory path should fail when we attempt to open it
    om.EnableCsv(true);
    REQUIRE_THROWS_AS(om.SetCsvPath(".", false), std::runtime_error);
}

// /* @brief Tests for OutputManager CSV exporting, including header writing and row formatting
// */
// TEST_CASE("CSV switching paths writes to both files correctly", "[output][csv][switch]")
// {
//     cse498::OutputManager om;
//     om.SetMinLogLevel(cse498::LogLevel::Info);

//     const std::string f1 = "test_log_1.csv";
//     const std::string f2 = "test_log_2.csv";

//     // start with file 1
//     om.SetCsvPath(f1, false);
//     om.EnableCsv(true);
//     om.Log(cse498::LogLevel::Info, cse498::LogCategory::System, "MessageOne");

//     // switch to file 2
//     om.SetCsvPath(f2, false);
//     om.Log(cse498::LogLevel::Error, cse498::LogCategory::AI, "MessageTwo");

//     om.EnableCsv(false);

//     // verify file 1
//     {
//         std::ifstream file(f1);
//         REQUIRE(file.is_open());

//         std::string line;
//         REQUIRE(std::getline(file, line));
//         CHECK(line == "Timestamp(ms),Level,Category,Message");

//         REQUIRE(std::getline(file, line));
//         CHECK(Contains(line, "INFO"));
//         CHECK(Contains(line, "SYSTEM"));
//         CHECK(Contains(line, "MessageOne"));

//         // should not contain second message
//         CHECK_FALSE(std::getline(file, line));
//     }

//     // verify file 2
//     {
//         std::ifstream file(f2);
//         REQUIRE(file.is_open());

//         std::string line;
//         REQUIRE(std::getline(file, line));
//         CHECK(line == "Timestamp(ms),Level,Category,Message");

//         REQUIRE(std::getline(file, line));
//         CHECK(Contains(line, "ERROR"));
//         CHECK(Contains(line, "AI"));
//         CHECK(Contains(line, "MessageTwo"));

//         CHECK_FALSE(std::getline(file, line));
//     }

//     // cleanup (optional but polite)
//     std::remove(f1.c_str());
//     std::remove(f2.c_str());
// }


// /* @brief Tests for OutputManager CSV exporting
// */
// TEST_CASE("CSV Exporting writes header and rows", "[output][csv]")
// {
//     cse498::OutputManager om;

//     // std::cout << "\n=== Testing CSV Export ===\n";
//     const std::string testFile = "test_log.csv";

//     om.SetMinLogLevel(cse498::LogLevel::Info);
//     om.SetCsvPath(testFile, false); // false = overwrite
//     om.EnableCsv(true);

//     om.Log(cse498::LogLevel::Info, cse498::LogCategory::System, "CSV Test Message 1");
//     om.Log(cse498::LogLevel::Error, cse498::LogCategory::AI, "CSV Test Message 2");

//     om.EnableCsv(false); // Close the CSV file to flush contents

//     std::ifstream file(testFile);
//     CHECK(file.is_open());

//     std::string line;

//     CHECK(std::getline(file, line));
//     CHECK(line == "Timestamp(ms),Level,Category,Message");

//     CHECK(std::getline(file, line));
//     CHECK(Contains(line, "INFO"));
//     CHECK(Contains(line, "SYSTEM"));
//     CHECK(Contains(line, "CSV Test Message 1"));

//     CHECK(std::getline(file, line));
//     CHECK(Contains(line, "ERROR"));
//     CHECK(Contains(line, "AI"));
//     CHECK(Contains(line, "CSV Test Message 2"));
// }