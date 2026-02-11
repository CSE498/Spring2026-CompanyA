
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../../source/Interfaces/GUI/ErrorManager.hpp"

#include <sstream>
#include <iostream>

// Helper to capture stderr output during a test
// Made via reference to Claude Sonnet 4.5
struct StderrCapture {
    std::streambuf* originalBuf;
    std::ostringstream captured;

    StderrCapture() {
        originalBuf = std::cerr.rdbuf(captured.rdbuf());
    }

    ~StderrCapture() {
        std::cerr.rdbuf(originalBuf);
    }

    std::string Get() const {
        return captured.str();
    }
};


TEST_CASE("Test ErrorManager Color Setters", "[ErrorManager]")
{
    cse498::ErrorManager em;

    CHECK_NOTHROW(em.SetFatalErrorColor(cse498::ErrorColor::BOLD));
    CHECK_NOTHROW(em.SetTerminalErrorColor(cse498::ErrorColor::RED));
    CHECK_NOTHROW(em.SetTerminalWarningColor(cse498::ErrorColor::YELLOW));
    CHECK_NOTHROW(em.SetLogErrorColor(cse498::ErrorColor::CYAN));
    CHECK_NOTHROW(em.SetLogWarningColor(cse498::ErrorColor::MAGENTA));

    // Custom ANSI code
    CHECK_NOTHROW(em.SetFatalErrorColor("\033[38;5;208m"));
    CHECK_NOTHROW(em.SetTerminalErrorColor("\033[38;5;208m"));
}


TEST_CASE("Test ErrorManager RaiseTerminalError Output", "[ErrorManager]")
{
    cse498::ErrorManager em;
    StderrCapture capture;

    // RaiseTerminalError should print and continue — program should NOT exit
    em.RaiseTerminalError("player walked into a wall");

    std::string output = capture.Get();
    CHECK(output.find("[ERROR]") != std::string::npos);
    CHECK(output.find("player walked into a wall") != std::string::npos);
}


TEST_CASE("Test ErrorManager RaiseTerminalError Does Not Stop Execution", "[ErrorManager]")
{
    cse498::ErrorManager em;
    StderrCapture capture;

    bool reachedAfterError = false;

    em.RaiseTerminalError("non-fatal error");
    reachedAfterError = true;

    // If RaiseTerminalError called std::exit this line would never be reached
    CHECK(reachedAfterError == true);
}


TEST_CASE("Test ErrorManager RaiseTerminalWarning Output", "[ErrorManager]")
{
    cse498::ErrorManager em;
    StderrCapture capture;

    em.RaiseTerminalWarning("low health warning");

    std::string output = capture.Get();
    CHECK(output.find("[WARNING]") != std::string::npos);
    CHECK(output.find("low health warning") != std::string::npos);
}


TEST_CASE("Test ErrorManager RaiseTerminalWarning Does Not Stop Execution", "[ErrorManager]")
{
    cse498::ErrorManager em;
    StderrCapture capture;

    bool reachedAfterWarning = false;

    em.RaiseTerminalWarning("non-fatal warning");
    reachedAfterWarning = true;

    CHECK(reachedAfterWarning == true);
}


TEST_CASE("Test ErrorManager LogError Output", "[ErrorManager]")
{
    cse498::ErrorManager em;
    StderrCapture capture;

    em.LogError("test log error");

    std::string output = capture.Get();
    CHECK(output.find("[LOG ERROR]") != std::string::npos);
    CHECK(output.find("test log error") != std::string::npos);
}


TEST_CASE("Test ErrorManager LogError Does Not Stop Execution", "[ErrorManager]")
{
    cse498::ErrorManager em;
    StderrCapture capture;

    bool reachedAfterLog = false;

    em.LogError("non-fatal log error");
    reachedAfterLog = true;

    CHECK(reachedAfterLog == true);
}


TEST_CASE("Test ErrorManager LogWarning Output", "[ErrorManager]")
{
    cse498::ErrorManager em;
    StderrCapture capture;

    em.LogWarning("test log warning");

    std::string output = capture.Get();
    CHECK(output.find("[LOG WARN]") != std::string::npos);
    CHECK(output.find("test log warning") != std::string::npos);
}


TEST_CASE("Test ErrorManager LogWarning Does Not Stop Execution", "[ErrorManager]")
{
    cse498::ErrorManager em;
    StderrCapture capture;

    bool reachedAfterLogWarning = false;

    em.LogWarning("non-fatal log warning");
    reachedAfterLogWarning = true;

    CHECK(reachedAfterLogWarning == true);
}

/// Should be able to clear the error log without issue - nothing currently saves to the log but if in time we do want
/// to track events would be helpful
TEST_CASE("Test ErrorManager ClearErrors", "[ErrorManager]")
{
    cse498::ErrorManager em;

    CHECK_NOTHROW(em.ClearErrors());
}


TEST_CASE("Test ErrorManager Custom Color Output", "[ErrorManager]")
{
    cse498::ErrorManager em;
    StderrCapture capture;

    em.SetLogErrorColor(cse498::ErrorColor::GREEN);
    em.LogError("custom color error");

    std::string output = capture.Get();
    CHECK(output.find("custom color error") != std::string::npos);
}


TEST_CASE("Test ErrorManager Multiple Non-Fatal Errors Continue Execution", "[ErrorManager]")
{
    cse498::ErrorManager em;
    StderrCapture capture;

    int executionCount = 0;

    em.RaiseTerminalError("first wall collision");
    executionCount++;
    em.RaiseTerminalError("second wall collision");
    executionCount++;
    em.RaiseTerminalWarning("low stamina");
    executionCount++;
    em.LogError("pathfinding failed");
    executionCount++;

    // All four calls should complete
    CHECK(executionCount == 4);

    std::string output = capture.Get();
    CHECK(output.find("first wall collision") != std::string::npos);
    CHECK(output.find("second wall collision") != std::string::npos);
    CHECK(output.find("low stamina") != std::string::npos);
    CHECK(output.find("pathfinding failed") != std::string::npos);
}


/// This function intentionally kills the program - this would interrupt the testing suite as well I believe.
/// For that reason, it's left here as a placeholder in case in the future a more meaningful test is thought of.
TEST_CASE("Test ErrorManager RaiseFatalError NOTE", "[ErrorManager]")
{
    CHECK(true);
}