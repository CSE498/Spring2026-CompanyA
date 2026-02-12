#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/OutputManager.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>
static bool Contains(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
}

/*@brief Captures anything written to std::cout during the lifetime of this object */
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

TEST_CASE("OutputManager::ShouldLog respects minimum log level", "[output][shouldlog]")
{
    OutputManager om;

    SECTION("Min = DEBUG allows DEBUG..ERROR")
    {
        om.SetMinLogLevel(LogLevel::DEBUG);

        REQUIRE(om.ShouldLog(LogLevel::DEBUG));
        REQUIRE(om.ShouldLog(LogLevel::Verbose));
        REQUIRE(om.ShouldLog(LogLevel::Info));
        REQUIRE(om.ShouldLog(LogLevel::Warn));
        REQUIRE(om.ShouldLog(LogLevel::Error));
    }

    SECTION("Min = Verbose filters DEBUG, allows Verbose..ERROR")
    {
        om.SetMinLogLevel(LogLevel::Verbose);

        REQUIRE_FALSE(om.ShouldLog(LogLevel::DEBUG));
        REQUIRE(om.ShouldLog(LogLevel::Verbose));
        REQUIRE(om.ShouldLog(LogLevel::Info));
        REQUIRE(om.ShouldLog(LogLevel::Warn));
        REQUIRE(om.ShouldLog(LogLevel::Error));
    }

    SECTION("Min = Info filters DEBUG/Verbose, allows Info..Error")
    {
        om.SetMinLogLevel(LogLevel::Info);

        REQUIRE_FALSE(om.ShouldLog(LogLevel::DEBUG));
        REQUIRE_FALSE(om.ShouldLog(LogLevel::Verbose));
        REQUIRE(om.ShouldLog(LogLevel::Info));
        REQUIRE(om.ShouldLog(LogLevel::Warn));
        REQUIRE(om.ShouldLog(LogLevel::Error));
    }

    SECTION("Min = Warn filters <Warn, allows Warn..Error")
    {
        om.SetMinLogLevel(LogLevel::Warn);

        REQUIRE_FALSE(om.ShouldLog(LogLevel::DEBUG));
        REQUIRE_FALSE(om.ShouldLog(LogLevel::Verbose));
        REQUIRE_FALSE(om.ShouldLog(LogLevel::Info));
        REQUIRE(om.ShouldLog(LogLevel::Warn));
        REQUIRE(om.ShouldLog(LogLevel::Error));
    }

    SECTION("Min = Error filters <Error, allows Error only")
    {
        om.SetMinLogLevel(LogLevel::Error);

        REQUIRE_FALSE(om.ShouldLog(LogLevel::DEBUG));
        REQUIRE_FALSE(om.ShouldLog(LogLevel::Verbose));
        REQUIRE_FALSE(om.ShouldLog(LogLevel::Info));
        REQUIRE_FALSE(om.ShouldLog(LogLevel::Warn));
        REQUIRE(om.ShouldLog(LogLevel::Error));
    }

    SECTION("Min = Silent filters everything (including DEBUG..ERROR)")
    {
        om.SetMinLogLevel(LogLevel::Silent);

        REQUIRE_FALSE(om.ShouldLog(LogLevel::DEBUG));
        REQUIRE_FALSE(om.ShouldLog(LogLevel::Verbose));
        REQUIRE_FALSE(om.ShouldLog(LogLevel::Info));
        REQUIRE_FALSE(om.ShouldLog(LogLevel::Warn));
        REQUIRE_FALSE(om.ShouldLog(LogLevel::Error));
    }
}

TEST_CASE("OutputManager::Log is filtered when below minimum level", "[output][log][filter]")
{
    OutputManager om;

    CoutCapture cap;
    om.SetMinLogLevel(LogLevel::Warn);
    om.Log(LogLevel::Info, LogCategory::System, "This should NOT print");

    REQUIRE(cap.Str().empty());
}

TEST_CASE("OutputManager::Log formats level/category/message (timestamps disabled)", "[output][log][format]")
{
    OutputManager om;
    om.SetMinLogLevel(LogLevel::Info);
    om.EnableTimestamps(false);

    SECTION("Combat category")
    {
        CoutCapture cap;
        om.Log(LogLevel::Info, LogCategory::Combat, "Goblin hit player");
        const std::string out = cap.Str();

        REQUIRE_FALSE(out.empty());
        REQUIRE(Contains(out, "[INFO]"));
        REQUIRE(Contains(out, "[COMBAT]"));
        REQUIRE(Contains(out, "Goblin hit player"));
        REQUIRE_FALSE(Contains(out, "ms]"));
    }

    SECTION("Puzzle category")
    {
        CoutCapture cap;
        om.Log(LogLevel::Info, LogCategory::Puzzle, "Player solved riddle");
        const std::string out = cap.Str();

        REQUIRE_FALSE(out.empty());
        REQUIRE(Contains(out, "[INFO]"));
        REQUIRE(Contains(out, "[PUZZLE]"));
        REQUIRE(Contains(out, "Player solved riddle"));
        REQUIRE_FALSE(Contains(out, "ms]"));
    }
}

TEST_CASE("OutputManager timestamp toggling affects output", "[output][log][timestamp]")
{
    OutputManager om;

    CoutCapture cap;
    om.SetMinLogLevel(LogLevel::Info);
    om.EnableTimestamps(true);
    om.Log(LogLevel::Info, LogCategory::System, "Startup");

    REQUIRE(Contains(cap.Str(), "ms]"));
}

TEST_CASE("OutputManager sink behavior (Step 3)", "[output][sink]")
{
    OutputManager om;
    std::vector<std::string> lines;

    // Quiet the test: route output to our vector sink
    om.ClearSinks();
    om.AddSink([&](std::string_view s) { lines.emplace_back(s); });

    SECTION("Filtered logs emit nothing")
    {
        lines.clear();
        om.SetMinLogLevel(LogLevel::Warn);
        om.EnableTimestamps(false);
        om.Log(LogLevel::Info, LogCategory::System, "Should not appear");

        REQUIRE(lines.empty());
    }

    SECTION("Allowed log emits exactly one formatted line")
    {
        lines.clear();
        om.SetMinLogLevel(LogLevel::Info);
        om.EnableTimestamps(false);
        om.Log(LogLevel::Info, LogCategory::Combat, "Goblin hit player");

        REQUIRE(lines.size() == 1);
        REQUIRE(Contains(lines[0], "[INFO]"));
        REQUIRE(Contains(lines[0], "[COMBAT]"));
        REQUIRE(Contains(lines[0], "Goblin hit player"));
        REQUIRE_FALSE(Contains(lines[0], "ms]"));
    }

    SECTION("Timestamp toggle affects sink output")
    {
        lines.clear();
        om.SetMinLogLevel(LogLevel::Info);
        om.EnableTimestamps(true);
        om.Log(LogLevel::Info, LogCategory::System, "Startup");

        REQUIRE(lines.size() == 1);
        REQUIRE(Contains(lines[0], "ms]"));
    }
}

TEST_CASE("Visual timestamp demo (prints to console)", "[output][manual][visual]")
{
    OutputManager om;

    std::cout << "\n=== Visual Timestamp Demo (should show ms) ===\n";

    // Force logs to the console for visual inspection
    om.ClearSinks();
    om.AddSink([](std::string_view s) {
        std::cout << s << "\n";
    });

    om.SetMinLogLevel(LogLevel::Info);
    om.EnableTimestamps(true);

    om.Log(LogLevel::Info, LogCategory::System, "Visual test: first message");
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    om.Log(LogLevel::Info, LogCategory::System, "Visual test: second message (later)");

    SUCCEED(); // This test is intentionally "visual"
}

TEST_CASE("CSV Exporting writes header and rows", "[output][csv]")
{
    OutputManager om;

    std::cout << "\n=== Testing CSV Export ===\n";
    const std::string testFile = "test_log.csv";

    om.SetMinLogLevel(LogLevel::Info);
    om.SetCsvPath(testFile, false); // false = overwrite
    om.EnableCsv(true);

    om.Log(LogLevel::Info, LogCategory::System, "CSV Test Message 1");
    om.Log(LogLevel::Error, LogCategory::AI, "CSV Test Message 2");

    // Encourage flushing/closing if your implementation does so on disable
    om.EnableCsv(false);

    std::ifstream file(testFile);
    REQUIRE(file.is_open());

    std::string line;

    REQUIRE(std::getline(file, line));
    REQUIRE(line == "Timestamp(ms),Level,Category,Message");

    REQUIRE(std::getline(file, line));
    REQUIRE(Contains(line, "INFO"));
    REQUIRE(Contains(line, "SYSTEM"));
    REQUIRE(Contains(line, "CSV Test Message 1"));

    REQUIRE(std::getline(file, line));
    REQUIRE(Contains(line, "ERROR"));
    REQUIRE(Contains(line, "AI"));
    REQUIRE(Contains(line, "CSV Test Message 2"));
}