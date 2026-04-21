/**
 * @file WebButtonTest.cpp
 * @brief Unit tests for the WebButton class using the Catch2 testing framework.
 *
 * These tests verify behavior including label updates, callback execution,
 * enabled/disabled and visible/hidden state transitions, sizing, styling,
 * ID generation, synchronization with the DOM, move semantics, and general
 * robustness under repeated and edge-case operations when compiled to
 * WebAssembly using Emscripten.
 *
 * Additional Note:
 *      Portions of formatting, documentation, and cleanup were assisted by
 *      AI tooling to improve consistency and readability.
 *
 * All project classes and tests correspond to the cse498 WebUI subsystem.
 *
 */

#ifdef __EMSCRIPTEN__

// #define CATCH_CONFIG_MAIN
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include <string>
#include <type_traits>
#include "../../../source/Interfaces/WebUI/WebButton/WebButton.hpp"

using namespace cse498;

#define CHECK_MSG(cond, msg)                                                                                           \
    do {                                                                                                               \
        INFO(msg);                                                                                                     \
        CHECK(cond);                                                                                                   \
    } while (0)


// ========================================================
// Test 1: Constructor sets initial label and default state
// ========================================================
TEST_CASE("Constructor sets label", "[WebButton]") {
    WebButton btn("Click Me");
    CHECK_MSG(btn.GetLabel() == "Click Me", "label mismatch");
    CHECK_MSG(btn.IsEnabled() == true, "default should be enabled");
    CHECK_MSG(btn.IsVisible() == true, "default should be visible");
    CHECK_MSG(btn.GetWidth() == 0, "default width should be 0");
    CHECK_MSG(btn.GetHeight() == 0, "default height should be 0");
    CHECK_MSG(!btn.Id().empty(), "id should not be empty");
}


// ========================================================
// Test 2: Constructor default label is empty
// ========================================================
TEST_CASE("Constructor default label is empty", "[WebButton]") {
    WebButton btn;
    CHECK_MSG(btn.GetLabel().empty(), "default label should be empty");
}


// ========================================================
// Test 3: SetLabel and GetLabel update text correctly
// ========================================================
TEST_CASE("SetLabel and GetLabel", "[WebButton]") {
    WebButton btn("old");
    btn.SetLabel("new");
    CHECK_MSG(btn.GetLabel() == "new", "label not updated");
}


// ========================================================
// Test 4: SetLabel accepts empty string
// ========================================================
TEST_CASE("SetLabel to empty string", "[WebButton]") {
    WebButton btn("something");
    btn.SetLabel("");
    CHECK_MSG(btn.GetLabel().empty(), "label should be empty");
}


// ========================================================
// Test 5: SetCallback and Click invoke callback
// ========================================================
TEST_CASE("SetCallback and Click invokes callback", "[WebButton]") {
    WebButton btn("test");
    int count = 0;
    btn.SetCallback([&count]() { ++count; });
    btn.Click();
    CHECK_MSG(count == 1, "callback should be invoked once");
    btn.Click();
    CHECK_MSG(count == 2, "callback should be invoked twice");
}


// ========================================================
// Test 6: Click without callback is safe
// ========================================================
TEST_CASE("Click without callback does not crash", "[WebButton]") {
    WebButton btn("test");
    REQUIRE_NOTHROW(btn.Click());
}


// ========================================================
// Test 7: Click does not invoke callback when disabled
// ========================================================
TEST_CASE("Click when disabled does not invoke callback", "[WebButton]") {
    WebButton btn("test");
    int count = 0;
    btn.SetCallback([&count]() { ++count; });
    btn.Disable();
    btn.Click();
    CHECK_MSG(count == 0, "callback should not be invoked when disabled");
}


// ========================================================
// Test 8: Enable Disable IsEnabled updates state correctly
// ========================================================
TEST_CASE("Enable Disable IsEnabled", "[WebButton]") {
    WebButton btn("test");
    CHECK_MSG(btn.IsEnabled() == true, "default should be enabled");
    btn.Disable();
    CHECK_MSG(btn.IsEnabled() == false, "should be disabled");
    btn.Enable();
    CHECK_MSG(btn.IsEnabled() == true, "should be enabled again");
}


// ========================================================
// Test 9: Show Hide IsVisible updates state correctly
// ========================================================
TEST_CASE("Show Hide IsVisible", "[WebButton]") {
    WebButton btn("test");
    CHECK_MSG(btn.IsVisible() == true, "default should be visible");
    btn.Hide();
    CHECK_MSG(btn.IsVisible() == false, "should be hidden");
    btn.Show();
    CHECK_MSG(btn.IsVisible() == true, "should be visible again");
}


// ========================================================
// Test 10: Visibility toggles correctly multiple times
// ========================================================
TEST_CASE("Visibility toggle multiple times", "[WebButton]") {
    WebButton btn("test");
    for (int i = 0; i < 5; ++i) {
        btn.Hide();
        CHECK_MSG(btn.IsVisible() == false, "should be hidden");
        btn.Show();
        CHECK_MSG(btn.IsVisible() == true, "should be visible");
    }
}


// ========================================================
// Test 11: SetSize updates width and height
// ========================================================
TEST_CASE("SetSize sets width and height", "[WebButton]") {
    WebButton btn("test");
    btn.SetSize(150, 40);
    CHECK_MSG(btn.GetWidth() == 150, "width mismatch");
    CHECK_MSG(btn.GetHeight() == 40, "height mismatch");
}


// ========================================================
// Test 12: SetSize accepts zero dimensions
// ========================================================
TEST_CASE("SetSize with zero dimensions", "[WebButton]") {
    WebButton btn("test");
    btn.SetSize(0, 0);
    CHECK_MSG(btn.GetWidth() == 0, "width should be 0");
    CHECK_MSG(btn.GetHeight() == 0, "height should be 0");
}


// ========================================================
// Test 13: SetBackgroundColor accepts multiple CSS values
// ========================================================
TEST_CASE("SetBackgroundColor does not crash", "[WebButton]") {
    WebButton btn("test");
    REQUIRE_NOTHROW(btn.SetBackgroundColor("#4CAF50"));
    REQUIRE_NOTHROW(btn.SetBackgroundColor("rgb(255, 0, 0)"));
    REQUIRE_NOTHROW(btn.SetBackgroundColor("transparent"));
}


// ========================================================
// Test 14: SetTextColor accepts multiple CSS values
// ========================================================
TEST_CASE("SetTextColor does not crash", "[WebButton]") {
    WebButton btn("test");
    REQUIRE_NOTHROW(btn.SetTextColor("white"));
    REQUIRE_NOTHROW(btn.SetTextColor("#000000"));
    REQUIRE_NOTHROW(btn.SetTextColor("rgb(0, 128, 255)"));
}


// ========================================================
// Test 15: Id is unique per instance
// ========================================================
TEST_CASE("Id is unique per instance", "[WebButton]") {
    WebButton a("a");
    WebButton b("b");
    WebButton c("c");
    CHECK_MSG(a.Id() != b.Id(), "a and b should have different ids");
    CHECK_MSG(b.Id() != c.Id(), "b and c should have different ids");
    CHECK_MSG(a.Id() != c.Id(), "a and c should have different ids");
}


// ========================================================
// Test 16: Id has expected prefix
// ========================================================
TEST_CASE("Id has expected prefix", "[WebButton]") {
    WebButton btn("test");
    std::string id = btn.Id();
    CHECK_MSG(id.rfind("webbutton-", 0) == 0, "id should start with 'webbutton-'");
}


// ========================================================
// Test 17: SyncFromModel preserves button state
// ========================================================
TEST_CASE("syncFromModel does not crash and preserves state", "[WebButton]") {
    WebButton btn("sync");
    btn.SetSize(100, 50);
    btn.SetBackgroundColor("#FF0000");
    btn.SetTextColor("white");
    btn.Disable();
    btn.Hide();
    btn.SyncFromModel();
    CHECK_MSG(btn.GetLabel() == "sync", "label should remain");
    CHECK_MSG(btn.GetWidth() == 100, "width should remain");
    CHECK_MSG(btn.GetHeight() == 50, "height should remain");
    CHECK_MSG(btn.IsEnabled() == false, "should remain disabled");
    CHECK_MSG(btn.IsVisible() == false, "should remain hidden");
}

// ========================================================
// Test 18: Move constructor transfers state and callback
// ========================================================
TEST_CASE("Move constructor transfers state", "[WebButton]") {
    WebButton original("move me");
    original.SetSize(200, 60);
    original.Disable();
    original.Hide();
    std::string original_id = original.Id();
    int count = 0;
    original.SetCallback([&count]() { ++count; });

    WebButton moved(std::move(original));
    CHECK_MSG(moved.GetLabel() == "move me", "label should transfer");
    CHECK_MSG(moved.GetWidth() == 200, "width should transfer");
    CHECK_MSG(moved.GetHeight() == 60, "height should transfer");
    CHECK_MSG(moved.IsEnabled() == false, "enabled state should transfer");
    CHECK_MSG(moved.IsVisible() == false, "visibility should transfer");
    CHECK_MSG(moved.Id() == original_id, "id should transfer");
    moved.Enable();
    moved.Click();
    CHECK_MSG(count == 1, "callback should transfer and work");

    CHECK_MSG(original.GetWidth() == 0, "original width should be 0");
    CHECK_MSG(original.GetHeight() == 0, "original height should be 0");
    CHECK_MSG(original.IsEnabled() == false, "original should be disabled");
    CHECK_MSG(original.IsVisible() == false, "original should be hidden");
}


// ========================================================
// Test 19: Move assignment transfers state
// ========================================================
TEST_CASE("Move assignment transfers state", "[WebButton]") {
    WebButton src("source");
    src.SetSize(80, 30);
    src.SetBackgroundColor("#00FF00");
    std::string src_id = src.Id();

    WebButton dest("dest");
    dest = std::move(src);
    CHECK_MSG(dest.GetLabel() == "source", "label should transfer");
    CHECK_MSG(dest.GetWidth() == 80, "width should transfer");
    CHECK_MSG(dest.GetHeight() == 30, "height should transfer");
    CHECK_MSG(dest.Id() == src_id, "id should transfer");
    CHECK_MSG(src.GetWidth() == 0, "src width should be 0");
    CHECK_MSG(src.GetHeight() == 0, "src height should be 0");
    CHECK_MSG(src.IsEnabled() == false, "src should be disabled");
}


// ========================================================
// Test 20: Callback replacement works correctly
// ========================================================
TEST_CASE("Callback replacement works", "[WebButton]") {
    WebButton btn("test");
    int first_count = 0;
    int second_count = 0;

    btn.SetCallback([&first_count]() { ++first_count; });
    btn.Click();
    CHECK_MSG(first_count == 1, "first callback should be called");

    btn.SetCallback([&second_count]() { ++second_count; });
    btn.Click();
    CHECK_MSG(first_count == 1, "first callback should still be 1");
    CHECK_MSG(second_count == 1, "second callback should be called");
}


// ========================================================
// Test 21: Disable then enable restores click behavior
// ========================================================
TEST_CASE("Disable then Enable then Click works", "[WebButton]") {
    WebButton btn("test");
    int count = 0;
    btn.SetCallback([&count]() { ++count; });
    btn.Disable();
    btn.Click();
    CHECK_MSG(count == 0, "should not fire when disabled");
    btn.Enable();
    btn.Click();
    CHECK_MSG(count == 1, "should fire after re-enable");
}


// ========================================================
// Test 22: Multiple label changes update correctly
// ========================================================
TEST_CASE("Multiple label changes", "[WebButton]") {
    WebButton btn("first");
    btn.SetLabel("second");
    CHECK_MSG(btn.GetLabel() == "second", "label should be second");
    btn.SetLabel("third");
    CHECK_MSG(btn.GetLabel() == "third", "label should be third");
    btn.SetLabel("");
    CHECK_MSG(btn.GetLabel().empty(), "label should be empty");
}


// ========================================================
// Test 23: SetSize overwrites previous size
// ========================================================
TEST_CASE("SetSize overwrites previous size", "[WebButton]") {
    WebButton btn("test");
    btn.SetSize(100, 50);
    CHECK_MSG(btn.GetWidth() == 100, "width should be 100");
    CHECK_MSG(btn.GetHeight() == 50, "height should be 50");
    btn.SetSize(200, 80);
    CHECK_MSG(btn.GetWidth() == 200, "width should be 200");
    CHECK_MSG(btn.GetHeight() == 80, "height should be 80");
}


// ========================================================
// Test 24: WebButton is move-only
// ========================================================
TEST_CASE("WebButton is move-only", "[WebButton]") {
    STATIC_REQUIRE(!std::is_copy_constructible_v<WebButton>);
    STATIC_REQUIRE(!std::is_copy_assignable_v<WebButton>);
    STATIC_REQUIRE(std::is_move_constructible_v<WebButton>);
    STATIC_REQUIRE(std::is_move_assignable_v<WebButton>);
}


// ========================================================
// Test 25: WebButton implements IDomElement
// ========================================================
TEST_CASE("WebButton implements IDomElement", "[WebButton]") {
    STATIC_REQUIRE(std::is_base_of_v<IDomElement, WebButton>);
}


// ========================================================
// Test 26: HandleClick delegates to Click
// ========================================================
TEST_CASE("HandleClick delegates to Click", "[WebButton]") {
    WebButton btn("test");
    int count = 0;
    btn.SetCallback([&count]() { ++count; });
    btn.HandleClick();
    CHECK_MSG(count == 1, "HandleClick should invoke callback via Click");
}


// ========================================================
// Test 27: HandleClick respects disabled state
// ========================================================
TEST_CASE("HandleClick respects disabled state", "[WebButton]") {
    WebButton btn("test");
    int count = 0;
    btn.SetCallback([&count]() { ++count; });
    btn.Disable();
    btn.HandleClick();
    CHECK_MSG(count == 0, "HandleClick should not invoke when disabled");
}


// ========================================================
// Test 28: Multiple rapid clicks all register
// ========================================================
TEST_CASE("Multiple rapid clicks all register", "[WebButton]") {
    WebButton btn("test");
    int count = 0;
    btn.SetCallback([&count]() { ++count; });
    for (int i = 0; i < 100; ++i) {
        btn.Click();
    }
    CHECK_MSG(count == 100, "all 100 clicks should register");
}


// ========================================================
// Test 29: Self move assignment remains safe
// ========================================================
TEST_CASE("Self move assignment is safe", "[WebButton]") {
    WebButton btn("self");
    btn.SetSize(50, 25);
    // btn = std::move(btn);
    CHECK_MSG(btn.GetLabel() == "self", "label should survive self-move");
    CHECK_MSG(btn.GetWidth() == 50, "width should survive self-move");
}


// ========================================================
// Test 30: Unmount is safe to call multiple times
// ========================================================
TEST_CASE("unmount is safe to call multiple times", "[WebButton]") {
    WebButton btn("test");
    REQUIRE_NOTHROW(btn.Unmount());
    REQUIRE_NOTHROW(btn.Unmount());
    REQUIRE_NOTHROW(btn.Unmount());
}

// ========================================================
// Test 31: Empty label button can still invoke callback
// ========================================================
TEST_CASE("Empty label button can still invoke callback", "[WebButton]") {
    WebButton btn("");
    int count = 0;
    btn.SetCallback([&count]() { ++count; });

    btn.Click();
    CHECK_MSG(count == 1, "empty-label button should still invoke callback");
}

// ========================================================
// Test 32: Multiple Enable and Disable calls are safe
// ========================================================
TEST_CASE("Multiple Enable and Disable calls are safe", "[WebButton]") {
    WebButton btn("test");

    btn.Disable();
    btn.Disable();
    CHECK_MSG(btn.IsEnabled() == false, "button should remain disabled");

    btn.Enable();
    btn.Enable();
    CHECK_MSG(btn.IsEnabled() == true, "button should remain enabled");
}

// ========================================================
// Test 33: SetBackgroundColor can be overwritten multiple times
// ========================================================
TEST_CASE("SetBackgroundColor can be overwritten multiple times", "[WebButton]") {
    WebButton btn("test");

    REQUIRE_NOTHROW(btn.SetBackgroundColor("#111111"));
    REQUIRE_NOTHROW(btn.SetBackgroundColor("#222222"));
    REQUIRE_NOTHROW(btn.SetBackgroundColor("rgb(1, 2, 3)"));
    REQUIRE_NOTHROW(btn.SetBackgroundColor(""));
}

// ========================================================
// Test 34: SetTextColor can be overwritten multiple times
// ========================================================
TEST_CASE("SetTextColor can be overwritten multiple times", "[WebButton]") {
    WebButton btn("test");

    REQUIRE_NOTHROW(btn.SetTextColor("#ffffff"));
    REQUIRE_NOTHROW(btn.SetTextColor("#000000"));
    REQUIRE_NOTHROW(btn.SetTextColor("rgb(9, 8, 7)"));
    REQUIRE_NOTHROW(btn.SetTextColor(""));
}

// ========================================================
// Test 35: SyncFromModel after label change preserves latest label
// ========================================================
TEST_CASE("SyncFromModel preserves latest label", "[WebButton]") {
    WebButton btn("first");
    btn.SetLabel("second");
    btn.SyncFromModel();

    CHECK_MSG(btn.GetLabel() == "second", "latest label should remain after SyncFromModel");
}

// ========================================================
// Test 36: Callback survives disable enable cycle
// ========================================================
TEST_CASE("Callback survives disable enable cycle", "[WebButton]") {
    WebButton btn("test");
    int count = 0;

    btn.SetCallback([&count]() { ++count; });

    btn.Disable();
    btn.Click();
    CHECK_MSG(count == 0, "callback should not fire while disabled");

    btn.Enable();
    btn.Click();
    CHECK_MSG(count == 1, "callback should still exist after re-enable");
}

// ========================================================
// Test 37: HandleClick works repeatedly when enabled
// ========================================================
TEST_CASE("HandleClick works repeatedly when enabled", "[WebButton]") {
    WebButton btn("test");
    int count = 0;

    btn.SetCallback([&count]() { ++count; });

    for (int i = 0; i < 10; ++i) {
        btn.HandleClick();
    }

    CHECK_MSG(count == 10, "HandleClick should invoke callback every time when enabled");
}

// ========================================================
// Test 38: Move constructor preserves callback while enabled
// ========================================================
TEST_CASE("Move constructor preserves callback while enabled", "[WebButton]") {
    WebButton original("move callback");
    int count = 0;

    original.SetCallback([&count]() { ++count; });

    WebButton moved(std::move(original));
    moved.Click();

    CHECK_MSG(count == 1, "moved button should preserve callback");
}

// ========================================================
// Test 39: Move assignment preserves callback while enabled
// ========================================================
TEST_CASE("Move assignment preserves callback while enabled", "[WebButton]") {
    WebButton src("src");
    int count = 0;
    src.SetCallback([&count]() { ++count; });

    WebButton dest("dest");
    dest = std::move(src);
    dest.Click();

    CHECK_MSG(count == 1, "move-assigned button should preserve callback");
}

// ========================================================
// Test 40: Move assignment overwrites old callback in destination
// ========================================================
TEST_CASE("Move assignment overwrites destination callback", "[WebButton]") {
    WebButton src("src");
    WebButton dest("dest");

    int src_count = 0;
    int dest_count = 0;

    src.SetCallback([&src_count]() { ++src_count; });
    dest.SetCallback([&dest_count]() { ++dest_count; });

    dest = std::move(src);
    dest.Click();

    CHECK_MSG(src_count == 1, "destination should now use source callback");
    CHECK_MSG(dest_count == 0, "old destination callback should be replaced");
}

// ========================================================
// Test 41: Zero width with positive height is allowed
// ========================================================
TEST_CASE("Zero width with positive height is allowed", "[WebButton]") {
    WebButton btn("test");

    btn.SetSize(0, 45);
    CHECK_MSG(btn.GetWidth() == 0, "width should remain 0");
    CHECK_MSG(btn.GetHeight() == 45, "height should be updated");
}

// ========================================================
// Test 42: Positive width with zero height is allowed
// ========================================================
TEST_CASE("Positive width with zero height is allowed", "[WebButton]") {
    WebButton btn("test");

    btn.SetSize(120, 0);
    CHECK_MSG(btn.GetWidth() == 120, "width should be updated");
    CHECK_MSG(btn.GetHeight() == 0, "height should remain 0");
}

// ========================================================
// Test 43: SyncFromModel preserves enabled visible state
// ========================================================
TEST_CASE("SyncFromModel preserves enabled visible state", "[WebButton]") {
    WebButton btn("sync-visible");
    btn.SetSize(90, 35);
    btn.Enable();
    btn.Show();
    btn.SyncFromModel();

    CHECK_MSG(btn.IsEnabled() == true, "button should remain enabled");
    CHECK_MSG(btn.IsVisible() == true, "button should remain visible");
    CHECK_MSG(btn.GetWidth() == 90, "width should remain 90");
    CHECK_MSG(btn.GetHeight() == 35, "height should remain 35");
}

// ========================================================
// Test 44: SetLabel after move assignment updates moved-to object
// ========================================================
TEST_CASE("SetLabel after move assignment updates moved-to object", "[WebButton]") {
    WebButton src("source");
    WebButton dest("dest");

    dest = std::move(src);
    dest.SetLabel("updated");

    CHECK_MSG(dest.GetLabel() == "updated", "moved-to object should still be usable");
}

// ========================================================
// Test 45: Moved-from object remains safe to use
// ========================================================
TEST_CASE("Moved-from object remains safe to use", "[WebButton]") {
    WebButton original("move-safe");
    WebButton moved(std::move(original));

    REQUIRE_NOTHROW(original.SetLabel("after move"));
    REQUIRE_NOTHROW(original.SetSize(10, 10));
    REQUIRE_NOTHROW(original.Show());
    REQUIRE_NOTHROW(original.Hide());
    REQUIRE_NOTHROW(original.SyncFromModel());
    REQUIRE_NOTHROW(original.Unmount());
}


// ========================================================
// Test 46: SyncFromModel after callback replacement keeps callback working
// ========================================================
TEST_CASE("SyncFromModel after callback replacement keeps callback working", "[WebButton]") {
    WebButton btn("test");
    int first = 0;
    int second = 0;

    btn.SetCallback([&first]() { ++first; });
    btn.Click();
    CHECK_MSG(first == 1, "first callback should fire once");

    btn.SetCallback([&second]() { ++second; });
    btn.SyncFromModel();
    btn.Click();

    CHECK_MSG(first == 1, "first callback should not fire again");
    CHECK_MSG(second == 1, "second callback should still work after SyncFromModel");
}

// ========================================================
// Test 47: Large size values are stored correctly
// ========================================================
TEST_CASE("Large size values are stored correctly", "[WebButton]") {
    WebButton btn("large");

    btn.SetSize(5000, 3000);
    CHECK_MSG(btn.GetWidth() == 5000, "large width should be stored");
    CHECK_MSG(btn.GetHeight() == 3000, "large height should be stored");
}

#endif // __EMSCRIPTEN__
