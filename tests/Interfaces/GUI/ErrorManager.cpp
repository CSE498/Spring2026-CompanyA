
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../../source/Interfaces/GUI/ErrorManager.hpp"

#include <iostream>
#include <sstream>

// Helper to capture stderr output during a test
// Made via reference to Claude Sonnet 4.5
struct StderrCapture {
    std::streambuf* originalBuf;
    std::ostringstream captured;

    StderrCapture() { originalBuf = std::cerr.rdbuf(captured.rdbuf()); }

    ~StderrCapture() { std::cerr.rdbuf(originalBuf); }

    std::string Get() const { return captured.str(); }
};


// /// Meant only to run see the GUIError function work, not meant for normal test suite running
// TEST_CASE("ErrorManager GUIError shows SDL popup without crashing", "[ErrorManager]") {
//
//     // SDL must be initialized for SDL_ShowSimpleMessageBox to work
//     REQUIRE(SDL_Init(0) == 0);
//
//     cse498::ErrorManager manager;
//
//     SECTION("GUIError with a basic message") {
//         SDL_ClearError();
//         manager.GUIError("Test error message");
//         std::string sdl_error = SDL_GetError();
//         CHECK(sdl_error.empty());
//     }
//
//     SECTION("GUIError with an empty message") {
//         SDL_ClearError();
//         manager.GUIError("");
//         std::string sdl_error = SDL_GetError();
//         CHECK(sdl_error.empty());
//     }
//
//     SDL_Quit();
// }


TEST_CASE("Test ErrorManager Color Setters", "[ErrorManager]") {
    cse498::ErrorManager em;

    em.SetFatalErrorColor(cse498::ErrorManager::ErrorColor::BOLD);
    CHECK(em.GetFatalErrorColor() == cse498::ErrorManager::ErrorColor::BOLD);

    em.SetTerminalErrorColor(cse498::ErrorManager::ErrorColor::RED);
    CHECK(em.GetTerminalErrorColor() == cse498::ErrorManager::ErrorColor::RED);

    em.SetTerminalWarningColor(cse498::ErrorManager::ErrorColor::YELLOW);
    CHECK(em.GetTerminalWarningColor() == cse498::ErrorManager::ErrorColor::YELLOW);

    em.SetLogErrorColor(cse498::ErrorManager::ErrorColor::CYAN);
    CHECK(em.GetLogErrorColor() == cse498::ErrorManager::ErrorColor::CYAN);

    em.SetLogWarningColor(cse498::ErrorManager::ErrorColor::MAGENTA);
    CHECK(em.GetLogWarningColor() == cse498::ErrorManager::ErrorColor::MAGENTA);

    // Custom ANSI code
    em.SetFatalErrorColor("\033[38;5;208m");
    CHECK(em.GetFatalErrorColor() == "\033[38;5;208m");

    em.SetTerminalErrorColor("\033[38;5;208m");
    CHECK(em.GetTerminalErrorColor() == "\033[38;5;208m");
}


TEST_CASE("Test ErrorManager RaiseTerminalError Output", "[ErrorManager]") {
    cse498::ErrorManager em;
    StderrCapture capture;

    // RaiseTerminalError should print and continue — program should NOT exit
    em.RaiseTerminalError("player walked into a wall");

    std::string output = capture.Get();
    CHECK(output.find("[ERROR]") != std::string::npos);
    CHECK(output.find("player walked into a wall") != std::string::npos);
}


TEST_CASE("Test ErrorManager RaiseTerminalError Does Not Stop Execution", "[ErrorManager]") {
    cse498::ErrorManager em;
    StderrCapture capture;

    bool reachedAfterError = false;

    em.RaiseTerminalError("non-fatal error");
    reachedAfterError = true;

    // If RaiseTerminalError called std::exit this line would never be reached
    CHECK(reachedAfterError == true);
}


TEST_CASE("Test ErrorManager RaiseTerminalWarning Output", "[ErrorManager]") {
    cse498::ErrorManager em;
    StderrCapture capture;

    em.RaiseTerminalWarning("low health warning");

    std::string output = capture.Get();
    CHECK(output.find("[WARNING]") != std::string::npos);
    CHECK(output.find("low health warning") != std::string::npos);
}


TEST_CASE("Test ErrorManager RaiseTerminalWarning Does Not Stop Execution", "[ErrorManager]") {
    cse498::ErrorManager em;
    StderrCapture capture;

    bool reachedAfterWarning = false;

    em.RaiseTerminalWarning("non-fatal warning");
    reachedAfterWarning = true;

    CHECK(reachedAfterWarning == true);
}


TEST_CASE("Test ErrorManager LogError Output", "[ErrorManager]") {
    cse498::ErrorManager em;
    StderrCapture capture;

    em.LogError("test log error");

    std::string output = capture.Get();
    CHECK(output.find("[LOG ERROR]") != std::string::npos);
    CHECK(output.find("test log error") != std::string::npos);
}


TEST_CASE("Test ErrorManager LogError Does Not Stop Execution", "[ErrorManager]") {
    cse498::ErrorManager em;
    StderrCapture capture;

    bool reachedAfterLog = false;

    em.LogError("non-fatal log error");
    reachedAfterLog = true;

    CHECK(reachedAfterLog == true);
}


TEST_CASE("Test ErrorManager LogWarning Output", "[ErrorManager]") {
    cse498::ErrorManager em;
    StderrCapture capture;

    em.LogWarning("test log warning");

    std::string output = capture.Get();
    CHECK(output.find("[LOG WARN]") != std::string::npos);
    CHECK(output.find("test log warning") != std::string::npos);
}


TEST_CASE("Test ErrorManager LogWarning Does Not Stop Execution", "[ErrorManager]") {
    cse498::ErrorManager em;
    StderrCapture capture;

    bool reachedAfterLogWarning = false;

    em.LogWarning("non-fatal log warning");
    reachedAfterLogWarning = true;

    CHECK(reachedAfterLogWarning == true);
}


TEST_CASE("Test ErrorManager Custom Color Output", "[ErrorManager]") {
    cse498::ErrorManager em;
    StderrCapture capture;

    em.SetLogErrorColor(cse498::ErrorManager::ErrorColor::GREEN);
    em.LogError("custom color error");

    std::string output = capture.Get();
    CHECK(output.find("custom color error") != std::string::npos);
}


TEST_CASE("Test ErrorManager Multiple Non-Fatal Errors Continue Execution", "[ErrorManager]") {
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
TEST_CASE("Test ErrorManager RaiseFatalError NOTE", "[ErrorManager]") { CHECK(true); }
