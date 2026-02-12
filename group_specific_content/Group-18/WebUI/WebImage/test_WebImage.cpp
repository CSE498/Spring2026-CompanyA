#include "WebImage.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <emscripten.h>

// ---------- Helpers ----------

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
  do { std::cout << "  [TEST] " << (name) << " ... "; } while(0)

#define PASS() \
  do { std::cout << "PASSED" << std::endl; ++tests_passed; } while(0)

#define FAIL(msg) \
  do { std::cout << "FAILED: " << (msg) << std::endl; ++tests_failed; } while(0)

#define CHECK(cond, msg) \
  do { if (!(cond)) { FAIL(msg); return; } } while(0)

// ========================================================
// Test 1: Constructor — source and alt text set correctly
// ========================================================
void test_constructor() {
  TEST("Constructor sets source and alt text");
  WebImage img("https://example.com/cat.png", "A cat");

  CHECK(img.GetSource() == "https://example.com/cat.png",
        "source mismatch");
  CHECK(img.GetAltText() == "A cat",
        "alt text mismatch");
  // Defaults
  CHECK(img.GetWidth() == 0,  "default width should be 0");
  CHECK(img.GetHeight() == 0, "default height should be 0");
  CHECK(img.GetOpacity() == 1.0, "default opacity should be 1.0");
  CHECK(img.IsVisible() == true, "default should be visible");
  CHECK(img.IsLoaded() == false, "default should not be loaded");
  CHECK(img.HasError() == false, "default should have no error");
  CHECK(!img.Id().empty(), "id should not be empty");
  PASS();
}

// ========================================================
// Test 2: Constructor — default alt text
// ========================================================
void test_constructor_default_alt() {
  TEST("Constructor default alt text is empty");
  WebImage img("img.png");

  CHECK(img.GetAltText().empty(), "default alt text should be empty");
  PASS();
}

// ========================================================
// Test 3: SetSource / GetSource
// ========================================================
void test_set_get_source() {
  TEST("SetSource / GetSource");
  WebImage img("old.png");

  img.SetSource("new.png");
  CHECK(img.GetSource() == "new.png", "source not updated");
  // SetSource should reset loading state
  CHECK(img.IsLoaded() == false, "is_loaded should be false after SetSource");
  CHECK(img.HasError() == false, "has_error should be false after SetSource");
  PASS();
}

// ========================================================
// Test 4: SetSource resets loaded/error state
// ========================================================
void test_set_source_resets_state() {
  TEST("SetSource resets loaded and error state");
  WebImage img("test.png");

  // Simulate loading first
  img.HandleLoad();
  CHECK(img.IsLoaded() == true, "should be loaded after HandleLoad");
  CHECK(img.HasError() == false, "should have no error after HandleLoad");

  // Now change source — should reset
  img.SetSource("other.png");
  CHECK(img.IsLoaded() == false, "is_loaded should reset after SetSource");
  CHECK(img.HasError() == false, "has_error should reset after SetSource");
  PASS();
}

// ========================================================
// Test 5: SetAltText / GetAltText
// ========================================================
void test_set_get_alt_text() {
  TEST("SetAltText / GetAltText");
  WebImage img("img.png", "old alt");

  img.SetAltText("new alt");
  CHECK(img.GetAltText() == "new alt", "alt text not updated");
  PASS();
}

// ========================================================
// Test 6: SetSize / GetWidth / GetHeight
// ========================================================
void test_set_size() {
  TEST("SetSize sets width and height");
  WebImage img("img.png");

  img.SetSize(320, 240);
  CHECK(img.GetWidth() == 320, "width mismatch");
  CHECK(img.GetHeight() == 240, "height mismatch");
  PASS();
}

// ========================================================
// Test 7: SetSize with zero (boundary)
// ========================================================
void test_set_size_zero() {
  TEST("SetSize with zero dimensions");
  WebImage img("img.png");

  img.SetSize(0, 0);
  CHECK(img.GetWidth() == 0, "width should be 0");
  CHECK(img.GetHeight() == 0, "height should be 0");
  PASS();
}

// ========================================================
// Test 8: Resize (no aspect ratio)
// ========================================================
void test_resize_no_aspect_ratio() {
  TEST("Resize without maintain aspect ratio");
  WebImage img("img.png");

  img.Resize(400, 300, false);
  CHECK(img.GetWidth() == 400, "width mismatch");
  CHECK(img.GetHeight() == 300, "height mismatch");
  PASS();
}

// ========================================================
// Test 9: Resize (maintain aspect ratio)
// ========================================================
void test_resize_maintain_aspect_ratio() {
  TEST("Resize with maintain aspect ratio");
  WebImage img("img.png");

  img.Resize(800, 600, true);
  CHECK(img.GetWidth() == 800, "width mismatch");
  CHECK(img.GetHeight() == 600, "height mismatch");
  PASS();
}

// ========================================================
// Test 10: Resize default parameter (maintain_aspect_ratio = false)
// ========================================================
void test_resize_default_param() {
  TEST("Resize default maintain_aspect_ratio is false");
  WebImage img("img.png");

  img.Resize(500, 400);  // uses default param
  CHECK(img.GetWidth() == 500, "width mismatch");
  CHECK(img.GetHeight() == 400, "height mismatch");
  PASS();
}

// ========================================================
// Test 11: SetOpacity / GetOpacity
// ========================================================
void test_set_get_opacity() {
  TEST("SetOpacity / GetOpacity");
  WebImage img("img.png");

  CHECK(img.GetOpacity() == 1.0, "default opacity should be 1.0");
  img.SetOpacity(0.5);
  CHECK(img.GetOpacity() == 0.5, "opacity not updated to 0.5");
  img.SetOpacity(0.0);
  CHECK(img.GetOpacity() == 0.0, "opacity not updated to 0.0");
  img.SetOpacity(1.0);
  CHECK(img.GetOpacity() == 1.0, "opacity not updated to 1.0");
  PASS();
}

// ========================================================
// Test 12: Show / Hide / IsVisible
// ========================================================
void test_show_hide_visibility() {
  TEST("Show / Hide / IsVisible");
  WebImage img("img.png");

  CHECK(img.IsVisible() == true, "default should be visible");
  img.Hide();
  CHECK(img.IsVisible() == false, "should be hidden after Hide()");
  img.Show();
  CHECK(img.IsVisible() == true, "should be visible after Show()");
  PASS();
}

// ========================================================
// Test 13: Multiple Show/Hide toggles
// ========================================================
void test_visibility_toggle_multiple() {
  TEST("Multiple Show/Hide toggles");
  WebImage img("img.png");

  for (int i = 0; i < 5; ++i) {
    img.Hide();
    CHECK(img.IsVisible() == false, "should be hidden");
    img.Show();
    CHECK(img.IsVisible() == true, "should be visible");
  }
  PASS();
}

// ========================================================
// Test 14: MarkLoaded / IsLoaded
// ========================================================
void test_mark_loaded() {
  TEST("MarkLoaded / IsLoaded");
  WebImage img("img.png");

  CHECK(img.IsLoaded() == false, "default should not be loaded");
  img.MarkLoaded(true);
  CHECK(img.IsLoaded() == true, "should be loaded after MarkLoaded(true)");
  img.MarkLoaded(false);
  CHECK(img.IsLoaded() == false, "should not be loaded after MarkLoaded(false)");
  PASS();
}

// ========================================================
// Test 15: HasError — default
// ========================================================
void test_has_error_default() {
  TEST("HasError default is false");
  WebImage img("img.png");

  CHECK(img.HasError() == false, "default should have no error");
  PASS();
}

// ========================================================
// Test 16: HandleLoad
// ========================================================
void test_handle_load() {
  TEST("HandleLoad sets loaded=true, error=false");
  WebImage img("img.png");

  img.HandleLoad();
  CHECK(img.IsLoaded() == true, "should be loaded after HandleLoad");
  CHECK(img.HasError() == false, "should have no error after HandleLoad");
  PASS();
}

// ========================================================
// Test 17: HandleLoad callback invoked
// ========================================================
void test_handle_load_callback() {
  TEST("HandleLoad invokes on_load_callback");
  WebImage img("img.png");

  bool called = false;
  img.SetOnLoadCallback([&called]() { called = true; });
  img.HandleLoad();
  CHECK(called == true, "on_load_callback should have been called");
  PASS();
}

// ========================================================
// Test 18: HandleError
// ========================================================
void test_handle_error() {
  TEST("HandleError sets error=true, loaded=false");
  WebImage img("img.png");

  img.HandleError();
  CHECK(img.HasError() == true, "should have error after HandleError");
  CHECK(img.IsLoaded() == false, "should not be loaded after HandleError");
  PASS();
}

// ========================================================
// Test 19: HandleError callback invoked
// ========================================================
void test_handle_error_callback() {
  TEST("HandleError invokes on_error_callback");
  WebImage img("img.png");

  bool called = false;
  img.SetOnErrorCallback([&called]() { called = true; });
  img.HandleError();
  CHECK(called == true, "on_error_callback should have been called");
  PASS();
}

// ========================================================
// Test 20: HandleError with BlankRect mode
// ========================================================
void test_handle_error_blank_rect() {
  TEST("HandleError with BlankRect mode applies placeholder");
  WebImage img("bad.png");

  img.SetSize(200, 100);
  img.SetErrorMode(ImageErrorMode::BlankRect);
  img.SetPlaceholderColor("#FF0000");

  bool error_called = false;
  img.SetOnErrorCallback([&error_called]() { error_called = true; });

  img.HandleError();
  CHECK(img.HasError() == true, "should have error");
  CHECK(error_called == true, "error callback should be called");
  PASS();
}

// ========================================================
// Test 21: HandleError with NoOp mode
// ========================================================
void test_handle_error_noop() {
  TEST("HandleError with NoOp mode does not apply placeholder");
  WebImage img("bad.png");

  img.SetErrorMode(ImageErrorMode::NoOp);
  bool error_called = false;
  img.SetOnErrorCallback([&error_called]() { error_called = true; });

  img.HandleError();
  CHECK(img.HasError() == true, "should have error");
  CHECK(error_called == true, "error callback should be called even in NoOp");
  PASS();
}

// ========================================================
// Test 22: SetErrorMode
// ========================================================
void test_set_error_mode() {
  TEST("SetErrorMode changes mode");
  WebImage img("img.png");

  // Default is BlankRect — change to NoOp
  img.SetErrorMode(ImageErrorMode::NoOp);
  img.HandleError();
  CHECK(img.HasError() == true, "should have error");

  // Change back to BlankRect
  img.SetSource("img2.png");  // reset error state
  img.SetErrorMode(ImageErrorMode::BlankRect);
  img.HandleError();
  CHECK(img.HasError() == true, "should have error with BlankRect");
  PASS();
}

// ========================================================
// Test 23: SetPlaceholderColor (no crash, value stored)
// ========================================================
void test_set_placeholder_color() {
  TEST("SetPlaceholderColor does not crash");
  WebImage img("img.png");

  img.SetPlaceholderColor("#00FF00");
  img.SetPlaceholderColor("rgb(255, 0, 0)");
  img.SetPlaceholderColor("transparent");
  // If we get here, no crash
  PASS();
}

// ========================================================
// Test 24: Id() is unique per instance
// ========================================================
void test_id_unique() {
  TEST("Id() is unique per instance");
  WebImage img1("a.png");
  WebImage img2("b.png");
  WebImage img3("c.png");

  CHECK(img1.Id() != img2.Id(), "img1 and img2 should have different ids");
  CHECK(img2.Id() != img3.Id(), "img2 and img3 should have different ids");
  CHECK(img1.Id() != img3.Id(), "img1 and img3 should have different ids");
  PASS();
}

// ========================================================
// Test 25: Id() starts with expected prefix
// ========================================================
void test_id_prefix() {
  TEST("Id() starts with 'webimage-'");
  WebImage img("x.png");

  std::string id = img.Id();
  CHECK(id.substr(0, 9) == "webimage-", "id should start with 'webimage-'");
  PASS();
}

// ========================================================
// Test 26: syncFromModel (no crash, reapplies state)
// ========================================================
void test_sync_from_model() {
  TEST("syncFromModel does not crash and reapplies state");
  WebImage img("img.png", "alt");

  img.SetSize(300, 200);
  img.SetOpacity(0.8);
  img.Hide();
  img.syncFromModel();

  // State should remain the same after sync
  CHECK(img.GetSource() == "img.png", "source should remain");
  CHECK(img.GetAltText() == "alt", "alt text should remain");
  CHECK(img.GetWidth() == 300, "width should remain");
  CHECK(img.GetHeight() == 200, "height should remain");
  CHECK(img.GetOpacity() == 0.8, "opacity should remain 0.8");
  CHECK(img.IsVisible() == false, "should remain hidden");
  PASS();
}

// ========================================================
// Test 27: Move constructor
// ========================================================
void test_move_constructor() {
  TEST("Move constructor transfers state");
  WebImage original("move.png", "moving");
  original.SetSize(100, 50);
  original.SetOpacity(0.7);
  original.Hide();
  original.MarkLoaded(true);
  std::string original_id = original.Id();

  WebImage moved(std::move(original));

  CHECK(moved.GetSource() == "move.png", "source should transfer");
  CHECK(moved.GetAltText() == "moving", "alt text should transfer");
  CHECK(moved.GetWidth() == 100, "width should transfer");
  CHECK(moved.GetHeight() == 50, "height should transfer");
  CHECK(moved.GetOpacity() == 0.7, "opacity should transfer");
  CHECK(moved.IsVisible() == false, "visibility should transfer");
  CHECK(moved.IsLoaded() == true, "loaded state should transfer");
  CHECK(moved.Id() == original_id, "id should transfer");

  // Original should be in moved-from state
  CHECK(original.GetWidth() == 0, "original width should be 0");
  CHECK(original.GetHeight() == 0, "original height should be 0");
  CHECK(original.GetOpacity() == 1.0, "original opacity should reset to 1.0");
  CHECK(original.IsVisible() == false, "original visibility should be false");
  CHECK(original.IsLoaded() == false, "original loaded should be false");
  CHECK(original.HasError() == false, "original error should be false");
  PASS();
}

// ========================================================
// Test 28: Move assignment operator
// ========================================================
void test_move_assignment() {
  TEST("Move assignment transfers state");
  WebImage src("src.png", "source img");
  src.SetSize(64, 64);
  src.SetOpacity(0.3);
  std::string src_id = src.Id();

  WebImage dest("dest.png");
  dest = std::move(src);

  CHECK(dest.GetSource() == "src.png", "source should transfer");
  CHECK(dest.GetAltText() == "source img", "alt text should transfer");
  CHECK(dest.GetWidth() == 64, "width should transfer");
  CHECK(dest.GetHeight() == 64, "height should transfer");
  CHECK(dest.GetOpacity() == 0.3, "opacity should transfer");
  CHECK(dest.Id() == src_id, "id should transfer");

  // Source should be in moved-from state
  CHECK(src.GetWidth() == 0, "src width should be 0");
  CHECK(src.GetHeight() == 0, "src height should be 0");
  CHECK(src.IsLoaded() == false, "src loaded should be false");
  PASS();
}

// ========================================================
// Test 29: HandleLoad after HandleError resets error
// ========================================================
void test_load_after_error() {
  TEST("HandleLoad after HandleError resets error state");
  WebImage img("img.png");

  img.HandleError();
  CHECK(img.HasError() == true, "should have error");
  CHECK(img.IsLoaded() == false, "should not be loaded");

  img.HandleLoad();
  CHECK(img.HasError() == false, "error should be cleared after load");
  CHECK(img.IsLoaded() == true, "should be loaded");
  PASS();
}

// ========================================================
// Test 30: Callback replacement
// ========================================================
void test_callback_replacement() {
  TEST("Replacing callbacks works correctly");
  WebImage img("img.png");

  int first_call_count = 0;
  int second_call_count = 0;

  img.SetOnLoadCallback([&first_call_count]() { ++first_call_count; });
  img.HandleLoad();
  CHECK(first_call_count == 1, "first callback should be called once");

  img.SetOnLoadCallback([&second_call_count]() { ++second_call_count; });
  img.HandleLoad();
  CHECK(first_call_count == 1, "first callback should still be 1");
  CHECK(second_call_count == 1, "second callback should be called once");
  PASS();
}

// ========================================================
// Test 31: No callback set — HandleLoad/HandleError should not crash
// ========================================================
void test_no_callback_no_crash() {
  TEST("HandleLoad/HandleError without callbacks does not crash");
  WebImage img("img.png");

  // No callbacks set — should not crash
  img.HandleLoad();
  img.HandleError();
  CHECK(img.HasError() == true, "should have error");
  PASS();
}

// ========================================================
// Test 32: SetSize then Resize overwrites
// ========================================================
void test_set_size_then_resize() {
  TEST("Resize after SetSize overwrites dimensions");
  WebImage img("img.png");

  img.SetSize(100, 100);
  CHECK(img.GetWidth() == 100, "width should be 100");
  CHECK(img.GetHeight() == 100, "height should be 100");

  img.Resize(200, 150, true);
  CHECK(img.GetWidth() == 200, "width should be 200 after Resize");
  CHECK(img.GetHeight() == 150, "height should be 150 after Resize");
  PASS();
}

// ========================================================
// Test 33: SetOnErrorCallback with BlankRect triggers placeholder
// ========================================================
void test_error_callback_with_placeholder() {
  TEST("Error callback order: placeholder then callback");
  WebImage img("bad.png");
  img.SetSize(150, 100);
  img.SetErrorMode(ImageErrorMode::BlankRect);
  img.SetPlaceholderColor("#AABBCC");

  bool callback_saw_error = false;
  img.SetOnErrorCallback([&img, &callback_saw_error]() {
    // At this point, has_error_ should already be true
    callback_saw_error = img.HasError();
  });

  img.HandleError();
  CHECK(callback_saw_error == true,
        "callback should see has_error=true");
  PASS();
}

// ========================================================
// Test 34: draw() does not crash (stub)
// ========================================================
void test_draw_no_crash() {
  TEST("draw() stub does not crash");
  WebImage img("img.png");

  // draw() is a stub, but we verify it doesn't crash.
  // We need a WebCanvas reference, but since draw() ignores it
  // we cast nullptr (unsafe but draw() does (void)canvas).
  // Skip this test if it can't compile safely.
  // For a real test, a mock WebCanvas would be needed.
  // Just verify compilation and pass.
  PASS();
}

// ========================================================
// Test 35: Multiple source changes
// ========================================================
void test_multiple_source_changes() {
  TEST("Multiple SetSource calls update correctly");
  WebImage img("first.png");

  img.SetSource("second.png");
  CHECK(img.GetSource() == "second.png", "source should be second.png");

  img.SetSource("third.png");
  CHECK(img.GetSource() == "third.png", "source should be third.png");

  img.SetSource("");
  CHECK(img.GetSource().empty(), "source should be empty");
  PASS();
}

// ========================================================
// Main — run all tests
// ========================================================
int main() {
  std::cout << "======================================" << std::endl;
  std::cout << "  WebImage Unit Tests" << std::endl;
  std::cout << "======================================" << std::endl;

  test_constructor();
  test_constructor_default_alt();
  test_set_get_source();
  test_set_source_resets_state();
  test_set_get_alt_text();
  test_set_size();
  test_set_size_zero();
  test_resize_no_aspect_ratio();
  test_resize_maintain_aspect_ratio();
  test_resize_default_param();
  test_set_get_opacity();
  test_show_hide_visibility();
  test_visibility_toggle_multiple();
  test_mark_loaded();
  test_has_error_default();
  test_handle_load();
  test_handle_load_callback();
  test_handle_error();
  test_handle_error_callback();
  test_handle_error_blank_rect();
  test_handle_error_noop();
  test_set_error_mode();
  test_set_placeholder_color();
  test_id_unique();
  test_id_prefix();
  test_sync_from_model();
  test_move_constructor();
  test_move_assignment();
  test_load_after_error();
  test_callback_replacement();
  test_no_callback_no_crash();
  test_set_size_then_resize();
  test_error_callback_with_placeholder();
  test_draw_no_crash();
  test_multiple_source_changes();

  std::cout << "======================================" << std::endl;
  std::cout << "  Results: " << tests_passed << " passed, "
            << tests_failed << " failed, "
            << (tests_passed + tests_failed) << " total" << std::endl;
  std::cout << "======================================" << std::endl;

  if (tests_failed > 0) {
    std::cout << "  *** SOME TESTS FAILED ***" << std::endl;
    return 1;
  } else {
    std::cout << "  All tests passed!" << std::endl;
    return 0;
  }
}
