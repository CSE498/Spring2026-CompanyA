/**
 * @file WebErrorManagerTest.cpp
 * @brief Unit tests for the WebErrorManager class using the Catch2 testing framework.
 *
 * These tests verify severity handling, non-fatal reporting behavior,
 * fatal throw behavior, browser alert integration under Emscripten,
 * and repeated call stability when compiled to WebAssembly.
 *
 *
 * Additional Note:
 *      Portions of formatting, documentation, and cleanup were assisted by
 *      AI tooling to improve consistency and readability.
 *
 * All project classes and tests correspond to the cse498 WebUI subsystem.
 */

#ifdef __EMSCRIPTEN__

#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include <emscripten.h>
#include <emscripten/val.h>
#include <stdexcept>
#include <string>
#include "../../../source/Interfaces/WebUI/WebErrorManager/WebErrorManager.hpp"

using cse498::WebErrorManager;
using emscripten::val;

// ---------- Helpers ----------

static val GetWindow() { return val::global("window"); }

static val GetConsole() { return val::global("console"); }

static void InstallAlertCapture() {
    val window = GetWindow();
    window.set("__test_lastAlert", std::string(""));
    window.set("__test_alertCount", 0);

    window.set("alert", val::global("captureAlert"));
}

EM_JS(void, captureAlertInstallShim, (), {
    globalThis.captureAlert = function(msg) {
        window.__test_lastAlert = String(msg);
        window.__test_alertCount = Number(window.__test_alertCount || 0) + 1;
    };
});

static std::string GetLastAlert() { return GetWindow()["__test_lastAlert"].as<std::string>(); }

static int GetAlertCount() { return GetWindow()["__test_alertCount"].as<int>(); }

static void ResetAlertCapture() {
    GetWindow().set("__test_lastAlert", std::string(""));
    GetWindow().set("__test_alertCount", 0);
}

// ---------- Map CHECK macro onto Catch2 ----------

#undef CHECK
#define CHECK(cond, msg)                                                                                               \
    do {                                                                                                               \
        INFO(msg);                                                                                                     \
        REQUIRE((cond));                                                                                               \
    } while (0)

// ========================================================
// Test 1: Info does not throw
// ========================================================
TEST_CASE("Info does not throw", "[WebErrorManager][info]") { REQUIRE_NOTHROW(WebErrorManager::Info("info test")); }

// ========================================================
// Test 2: Warning does not throw
// ========================================================
TEST_CASE("Warning does not throw", "[WebErrorManager][warning]") {
    REQUIRE_NOTHROW(WebErrorManager::Warning("warning test"));
}

// ========================================================
// Test 3: Error does not throw by default
// ========================================================
TEST_CASE("Error does not throw by default", "[WebErrorManager][error]") {
    REQUIRE_NOTHROW(WebErrorManager::Error("error test"));
}

// ========================================================
// Test 4: Fatal throws when configured
// ========================================================
TEST_CASE("Fatal throws when throwOnFatal is enabled", "[WebErrorManager][fatal]") {
    WebErrorManager::Options opts;
    opts.throwOnFatal = true;

    REQUIRE_THROWS_AS(WebErrorManager::Fatal("fatal throw test", opts), std::runtime_error);
}

// ========================================================
// Test 5: Fatal exception contains message text
// ========================================================
TEST_CASE("Fatal thrown exception contains message", "[WebErrorManager][fatal]") {
    WebErrorManager::Options opts;
    opts.throwOnFatal = true;

    try {
        WebErrorManager::Fatal("fatal message body", opts);
        FAIL("Expected std::runtime_error");
    } catch (const std::runtime_error& ex) {
        std::string msg = ex.what();
        CHECK(msg.find("fatal message body") != std::string::npos, "runtime_error should include original message");
    }
}

// ========================================================
// Test 6: Error with alert triggers browser alert capture
// ========================================================
TEST_CASE("Error with showAlert triggers alert", "[WebErrorManager][alert]") {
    captureAlertInstallShim();
    InstallAlertCapture();
    ResetAlertCapture();

    WebErrorManager::Options opts;
    opts.showAlert = true;

    WebErrorManager::Error("alertable error", opts);

    CHECK(GetAlertCount() == 1, "alert should be called once");
    CHECK(GetLastAlert().find("alertable error") != std::string::npos, "alert text should contain original message");
}

// ========================================================
// Test 7: Warning with alert triggers browser alert capture
// ========================================================
TEST_CASE("Warning with showAlert triggers alert", "[WebErrorManager][alert]") {
    captureAlertInstallShim();
    InstallAlertCapture();
    ResetAlertCapture();

    WebErrorManager::Options opts;
    opts.showAlert = true;

    WebErrorManager::Warning("warning alert text", opts);

    CHECK(GetAlertCount() == 1, "alert should be called once for warning");
    CHECK(GetLastAlert().find("warning alert text") != std::string::npos, "alert text should contain warning message");
}

// ========================================================
// Test 8: No alert when showAlert is false
// ========================================================
TEST_CASE("No alert when showAlert is false", "[WebErrorManager][alert]") {
    captureAlertInstallShim();
    InstallAlertCapture();
    ResetAlertCapture();

    WebErrorManager::Options opts;
    opts.showAlert = false;

    WebErrorManager::Error("silent browser error", opts);

    CHECK(GetAlertCount() == 0, "alert should not be called");
    CHECK(GetLastAlert().empty(), "last alert text should remain empty");
}

// ========================================================
// Test 9: Repeated error calls remain stable
// ========================================================
TEST_CASE("Repeated Error calls remain stable", "[WebErrorManager][repeat]") {
    for (int i = 0; i < 10; ++i) {
        REQUIRE_NOTHROW(WebErrorManager::Error("repeat error " + std::to_string(i)));
    }
}

// ========================================================
// Test 10: Repeated warning calls remain stable
// ========================================================
TEST_CASE("Repeated Warning calls remain stable", "[WebErrorManager][repeat]") {
    for (int i = 0; i < 10; ++i) {
        REQUIRE_NOTHROW(WebErrorManager::Warning("repeat warning " + std::to_string(i)));
    }
}

// ========================================================
// Test 11: Info with alert also triggers capture
// ========================================================
TEST_CASE("Info with showAlert triggers alert", "[WebErrorManager][info][alert]") {
    captureAlertInstallShim();
    InstallAlertCapture();
    ResetAlertCapture();

    WebErrorManager::Options opts;
    opts.showAlert = true;

    WebErrorManager::Info("info alert", opts);

    CHECK(GetAlertCount() == 1, "alert should be called once for info");
    CHECK(GetLastAlert().find("info alert") != std::string::npos, "alert text should contain info message");
}

// ========================================================
// Test 12: Fatal with alert and throw both triggers alert first
// ========================================================
TEST_CASE("Fatal with showAlert and throwOnFatal alerts and throws", "[WebErrorManager][fatal][alert]") {
    captureAlertInstallShim();
    InstallAlertCapture();
    ResetAlertCapture();

    WebErrorManager::Options opts;
    opts.showAlert = true;
    opts.throwOnFatal = true;

    try {
        WebErrorManager::Fatal("fatal alert throw", opts);
        FAIL("Expected std::runtime_error");
    } catch (const std::runtime_error& ex) {
        std::string msg = ex.what();
        CHECK(msg.find("fatal alert throw") != std::string::npos, "thrown exception should contain fatal message");
    }

    CHECK(GetAlertCount() == 1, "alert should be called before throw");
    CHECK(GetLastAlert().find("fatal alert throw") != std::string::npos, "alert should contain fatal message");
}

// ========================================================
// Test 13: Empty message is handled safely
// ========================================================
TEST_CASE("Empty message is handled safely", "[WebErrorManager][edge]") {
    REQUIRE_NOTHROW(WebErrorManager::Error(""));

    REQUIRE_NOTHROW(WebErrorManager::Warning(""));

    REQUIRE_NOTHROW(WebErrorManager::Info(""));
}

// ========================================================
// Test 14: Long message is handled safely
// ========================================================
TEST_CASE("Long message is handled safely", "[WebErrorManager][edge]") {
    std::string longMsg(5000, 'X');

    REQUIRE_NOTHROW(WebErrorManager::Error(longMsg));
}

// ========================================================
// Test 15: Browser console object exists for Emscripten path
// ========================================================
TEST_CASE("Browser console exists", "[WebErrorManager][emscripten]") {
    val console = GetConsole();
    CHECK((!console.isNull() && !console.isUndefined()), "global console object should exist");
}

#endif // __EMSCRIPTEN__
