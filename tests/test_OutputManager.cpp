#include "OutputManager.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>

static int num_passed = 0;
static int num_failed = 0;

static void Check(bool condition, const std::string& test_name)
{
    if (condition)
    {
        std::cerr << "PASS: " << test_name << "\n";
        num_passed++;
    }
    else
    {
        std::cerr << "FAIL: " << test_name << "\n";
        num_failed++;
    }
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

static bool Contains(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
}

int main()
{
    OutputManager om;

    // -------------------------
    // Test Group 1: ShouldLog behavior
    // -------------------------

    //setting to debug
    om.SetMinLogLevel(LogLevel::DEBUG);

    Check( om.ShouldLog(LogLevel::DEBUG),   "Debug min-level allows Debug");
    Check( om.ShouldLog(LogLevel::Verbose), "Debug min-level allows Verbose");
    Check( om.ShouldLog(LogLevel::Info),    "Debug min-level allows Info");
    Check( om.ShouldLog(LogLevel::Warn),    "Debug min-level allows Warn");
    Check( om.ShouldLog(LogLevel::Error),   "Debug min-level allows Error");

    std::cout << "\nChanging minimum log level...\n" << std::endl;

    
    //setting to verbose
    om.SetMinLogLevel(LogLevel::Verbose);

    Check(!om.ShouldLog(LogLevel::DEBUG),   "Verbose min-level filters Debug");
    Check( om.ShouldLog(LogLevel::DEBUG),   "Verbose min-level allows Debug (this should fail on purpose to show filtering)");
    Check( om.ShouldLog(LogLevel::Verbose), "Verbose min-level allows Verbose");
    Check( om.ShouldLog(LogLevel::Info),    "Verbose min-level allows Info");
    Check( om.ShouldLog(LogLevel::Warn),    "Verbose min-level allows Warn");
    Check( om.ShouldLog(LogLevel::Error),   "Verbose min-level allows Error");

    std::cout << "\nChanging minimum log level...\n" << std::endl;

    //setting to info
    om.SetMinLogLevel(LogLevel::Info);

    Check(!om.ShouldLog(LogLevel::DEBUG),    "Info min-level filters Debug");
    Check(!om.ShouldLog(LogLevel::Verbose),  "Info min-level filters Verbose");
    Check( om.ShouldLog(LogLevel::Info),     "Info min-level allows Info");
    Check( om.ShouldLog(LogLevel::Warn),     "Info min-level allows Warn");
    Check( om.ShouldLog(LogLevel::Error),    "Info min-level allows Error");

    std::cout << "\nChanging minimum log level...\n" << std::endl;


    //setting to warn
    om.SetMinLogLevel(LogLevel::Warn);
    Check(!om.ShouldLog(LogLevel::DEBUG),    "Warn min-level filters Debug");
    Check(!om.ShouldLog(LogLevel::Verbose),  "Warn min-level filters Verbose");
    Check(!om.ShouldLog(LogLevel::Info),     "Warn min-level filters Info");
    Check( om.ShouldLog(LogLevel::Warn),     "Warn min-level allows Warn");
    Check( om.ShouldLog(LogLevel::Error),    "Warn min-level allows Error");
    
    std::cout << "\nChanging minimum log level...\n" << std::endl;

    //setting to error
    om.SetMinLogLevel(LogLevel::Error);
    Check(!om.ShouldLog(LogLevel::DEBUG),    "Error min-level filters Debug");
    Check(!om.ShouldLog(LogLevel::Verbose),  "Error min-level filters Verbose");
    Check(!om.ShouldLog(LogLevel::Info),     "Error min-level filters Info");
    Check(!om.ShouldLog(LogLevel::Warn),     "Error min-level filters Warn");
    Check( om.ShouldLog(LogLevel::Error),    "Error min-level allows Error");

    std::cout << "\nChanging minimum log level...\n" << std::endl;

    //silent should not log anything regardless of min level
    Check(!om.ShouldLog(LogLevel::Silent),   "Silent level should not log anything (this should fail on purpose to show silent level allows nothing)");

    om.SetMinLogLevel(LogLevel::Silent);
    Check(!om.ShouldLog(LogLevel::DEBUG),    "Silent min-level filters Debug");
    Check(!om.ShouldLog(LogLevel::Verbose),  "Silent min-level filters Verbose");
    Check(!om.ShouldLog(LogLevel::Info),     "Silent min-level filters Info");
    Check(!om.ShouldLog(LogLevel::Warn),     "Silent min-level filters Warn");
    Check(!om.ShouldLog(LogLevel::Error),    "Silent min-level filters Error");

    std::cout << "\nChanging minimum log level...\n" << std::endl;


    // -------------------------
    // Test Group 2: Log() is filtered (no output)
    // -------------------------
    {
        CoutCapture cap;
        om.SetMinLogLevel(LogLevel::Warn);
        om.Log(LogLevel::Info, LogCategory::System, "This should NOT print");
        Check(cap.Str().empty(), "Log() produces no output when filtered out");
    }

    // -------------------------
    // Test Group 3: Log() prints level/category/message
    // -------------------------
    {
        CoutCapture cap;
        om.SetMinLogLevel(LogLevel::Info);
        om.EnableTimestamps(false);
        om.Log(LogLevel::Info, LogCategory::Combat, "Goblin hit player");
        std::string out = cap.Str();

        Check(!out.empty(), "Log() produces output when allowed");
        Check(Contains(out, "[INFO]"), "Log() output includes [INFO]");
        Check(Contains(out, "[COMBAT]"), "Log() output includes [COMBAT]");
        Check(Contains(out, "Goblin hit player"), "Log() output includes the message");
        Check(!Contains(out, "ms]"), "Timestamps disabled removes [###ms] prefix");
    }
    std::cout << "\nChanging category...\n" << std::endl;

    {
        CoutCapture cap;
        om.SetMinLogLevel(LogLevel::Info);
        om.EnableTimestamps(false);
        om.Log(LogLevel::Info, LogCategory::Puzzle , "Player solved riddle");
        std::string out = cap.Str();

        Check(!out.empty(), "Log() produces output when allowed");
        Check(Contains(out, "[INFO]"), "Log() output includes [INFO]");
        Check(Contains(out, "[PUZZLE]"), "Log() output includes [PUZZLE]");
        Check(Contains(out, "Player solved riddle"), "Log() output includes the message");
        Check(!Contains(out, "ms]"), "Timestamps disabled removes [###ms] prefix");
    }
    std::cout << "\nChanging category...\n" << std::endl;

    // -------------------------
    // Test Group 4: Timestamp toggling
    // -------------------------
    {
        CoutCapture cap;
        om.EnableTimestamps(true);
        om.SetMinLogLevel(LogLevel::Info);
        om.Log(LogLevel::Info, LogCategory::System, "Startup");
        std::string out = cap.Str();

        Check(Contains(out, "ms]"), "Timestamps enabled includes [###ms] prefix");
    }

    // -------------------------
    // Test Group 5: Sink behavior (Step 3)
    // -------------------------
    {
        std::vector<std::string> lines;

        // Replace default console output so test is quiet
        om.ClearSinks();
        om.AddSink([&](std::string_view s) 
        {
            lines.emplace_back(s);
        });

        // -------- SECTION 1: Filtered logs emit nothing --------
        lines.clear();
        om.SetMinLogLevel(LogLevel::Warn);
        om.EnableTimestamps(false);
        om.Log(LogLevel::Info, LogCategory::System, "Should not appear");

        Check(lines.empty(), "Sinks: filtered logs emit nothing");

        // -------- SECTION 2: Allowed log emits one formatted line --------
        lines.clear();
        om.SetMinLogLevel(LogLevel::Info);
        om.EnableTimestamps(false);
        om.Log(LogLevel::Info, LogCategory::Combat, "Goblin hit player");

        Check(lines.size() == 1, "Sinks: allowed log emits exactly one line");
        if (lines.size() == 1)
        {
            Check(Contains(lines[0], "[INFO]"), "Sinks: line includes [INFO]");
            Check(Contains(lines[0], "[COMBAT]"), "Sinks: line includes [COMBAT]");
            Check(Contains(lines[0], "Goblin hit player"), "Sinks: line includes message");
            Check(!Contains(lines[0], "ms]"), "Sinks: timestamps disabled removes [###ms]");
        }

        // -------- SECTION 3: Timestamp toggle affects output --------
        lines.clear();
        om.SetMinLogLevel(LogLevel::Info);
        om.EnableTimestamps(true);
        om.Log(LogLevel::Info, LogCategory::System, "Startup");

        Check(lines.size() == 1, "Sinks: timestamp-enabled emits exactly one line");
        if (lines.size() == 1)
        {
            Check(Contains(lines[0], "ms]"), "Sinks: timestamps enabled includes [###ms]");
        }
    // -------------------------
    // Test Group 6: Visual timestamp output (manual/visible)
    // -------------------------
    {
        std::cout << "\n=== Visual Timestamp Demo (should show ms) ===\n";

        // If you are using sinks (Step 3), this ensures logs go to the console.
        // If your OutputManager still prints directly to cout internally, this is still harmless.
        om.ClearSinks();
        om.AddSink([](std::string_view s) {
            std::cout << s << "\n";
        });

        om.SetMinLogLevel(LogLevel::Info);
        om.EnableTimestamps(true);

        // You SHOULD see something like: [0ms] [INFO] [SYSTEM] ...
        om.Log(LogLevel::Info, LogCategory::System, "Visual test: first message");

        // Delay so the second message has a larger ms value
        std::this_thread::sleep_for(std::chrono::milliseconds(25));

        // You SHOULD see something like: [25ms] [INFO] [SYSTEM] ...
        om.Log(LogLevel::Info, LogCategory::System, "Visual test: second message (later)");
    }

    // -------------------------
    // Test Group 7: CSV Exporting
    // -------------------------
    {
        std::cout << "\n=== Testing CSV Export ===\n";
        std::string testFile = "test_log.csv";
        
        om.SetMinLogLevel(LogLevel::Info);
        om.SetCsvPath(testFile, false); // false = overwrite to start fresh
        om.EnableCsv(true);

        om.Log(LogLevel::Info, LogCategory::System, "CSV Test Message 1");
        om.Log(LogLevel::Error, LogCategory::AI, "CSV Test Message 2");

        // Close the manager's file handle by setting path to empty or destroying object
        // For this test, we'll just read the file. 
        // Note: In production, you might want a specific Close() method.
        
        std::ifstream file(testFile);
        Check(file.is_open(), "CSV file was created");

        std::string line;
        std::getline(file, line);
        Check(line == "Timestamp(ms),Level,Category,Message", "CSV header written correctly");

        std::getline(file, line);
        Check(Contains(line, "INFO") && Contains(line, "SYSTEM") && Contains(line, "CSV Test Message 1"), "CSV Row 1 contains correct data");

        std::getline(file, line);
        Check(Contains(line, "ERROR") && Contains(line, "AI") && Contains(line, "CSV Test Message 2"), "CSV Row 2 contains correct data");
        
        file.close();
    }

    // -------------------------
    // Test Group 8: CSV Exporting to Specific Folder
    // -------------------------
    {
        std::cout << "\n=== Testing CSV Export to Local Folder ===\n";
        
        // Define your specific path
        std::string testFile = R"(C:\Users\tyler\OneDrive\Documents\CSE 498\test_log.csv)";
        
        om.SetMinLogLevel(LogLevel::Info);
        om.SetCsvPath(testFile, false); // false = overwrite to start fresh
        om.EnableCsv(true);

        om.Log(LogLevel::Info, LogCategory::System, "CSV Local Path Test 1");
        om.Log(LogLevel::Error, LogCategory::AI, "CSV Local Path Test 2");

        // Verification
        std::ifstream file(testFile);
        Check(file.is_open(), R"(CSV file was created at C:\Users\tyler...)");
        
        std::string line;
        std::getline(file, line);
        Check(line == "Timestamp(ms),Level,Category,Message", "CSV header written correctly");

        std::getline(file, line);
        Check(Contains(line, "INFO") && Contains(line, "SYSTEM") && Contains(line, "CSV Local Path Test 1"), "Row 1 verified");

        file.close();
    }

    // -------------------------
    // Summary
    // -------------------------
    std::cout << "\nSummary: " << num_passed << " passed, " << num_failed << " failed.\n";
    return (num_failed == 0) ? 0 : 1;
    }
}