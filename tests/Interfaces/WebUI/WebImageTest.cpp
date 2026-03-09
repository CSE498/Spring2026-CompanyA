#ifdef __EMSCRIPTEN__

#define CATCH_CONFIG_MAIN
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../../source/Interfaces/WebUI/WebImage/WebImage.hpp"
#include <string>

using namespace cse498;

// ---------- Helpers ----------

#define TEST(name) \
  do { INFO(name); } while(0)

#define PASS() \
  do {} while(0)

#define CHECK_MSG(cond, msg) \
  do { INFO(msg); CHECK(cond); } while(0)

#define REQUIRE_MSG(cond, msg) \
  do { INFO(msg); REQUIRE(cond); } while(0)

// ========================================================
// Test 1: Constructor — source and alt text set correctly
// ========================================================
void test_constructor() {
  TEST("Constructor sets source and alt text");
  WebImage img("https://example.com/cat.png", "A cat");

  REQUIRE_MSG(img.GetSource() == "https://example.com/cat.png",
        "source mismatch");
  REQUIRE_MSG(img.GetAltText() == "A cat",
        "alt text mismatch");
  CHECK_MSG(img.GetWidth() == 0,  "default width should be 0");
  CHECK_MSG(img.GetHeight() == 0, "default height should be 0");
  CHECK_MSG(img.GetOpacity() == 1.0, "default opacity should be 1.0");
  CHECK_MSG(img.IsVisible() == true, "default should be visible");
  CHECK_MSG(img.IsLoaded() == false, "default should not be loaded");
  CHECK_MSG(img.HasError() == false, "default should have no error");
  CHECK_MSG(!img.Id().empty(), "id should not be empty");
  PASS();
}

// ========================================================
// Test 2: Constructor — default alt text
// ========================================================
void test_constructor_default_alt() {
  TEST("Constructor default alt text is empty");
  WebImage img("img.png");

  REQUIRE_MSG(img.GetAltText().empty(), "default alt text should be empty");
  PASS();
}

// ========================================================
// Test 3: SetSource / GetSource
// ========================================================
void test_set_get_source() {
  TEST("SetSource / GetSource");
  WebImage img("old.png");

  img.SetSource("new.png");
  REQUIRE_MSG(img.GetSource() == "new.png", "source not updated");
  CHECK_MSG(img.IsLoaded() == false, "is_loaded should be false after SetSource");
  CHECK_MSG(img.HasError() == false, "has_error should be false after SetSource");
  PASS();
}

// ========================================================
// Test 4: SetSource resets loaded/error state
// ========================================================
void test_set_source_resets_state() {
  TEST("SetSource resets loaded and error state");
  WebImage img("test.png");

  img.HandleLoad();
  REQUIRE_MSG(img.IsLoaded() == true, "should be loaded after HandleLoad");
  CHECK_MSG(img.HasError() == false, "should have no error after HandleLoad");

  img.SetSource("other.png");
  CHECK_MSG(img.IsLoaded() == false, "is_loaded should reset after SetSource");
  CHECK_MSG(img.HasError() == false, "has_error should reset after SetSource");
  PASS();
}

// ========================================================
// Test 5: SetAltText / GetAltText
// ========================================================
void test_set_get_alt_text() {
  TEST("SetAltText / GetAltText");
  WebImage img("img.png", "old alt");

  img.SetAltText("new alt");
  REQUIRE_MSG(img.GetAltText() == "new alt", "alt text not updated");
  PASS();
}

// ========================================================
// Test 6: SetSize / GetWidth / GetHeight
// ========================================================
void test_set_size() {
  TEST("SetSize sets width and height");
  WebImage img("img.png");

  img.SetSize(320, 240);
  REQUIRE_MSG(img.GetWidth() == 320, "width mismatch");
  REQUIRE_MSG(img.GetHeight() == 240, "height mismatch");
  PASS();
}

// ========================================================
// Test 7: SetSize with zero (boundary)
// ========================================================
void test_set_size_zero() {
  TEST("SetSize with zero dimensions");
  WebImage img("img.png");

  img.SetSize(0, 0);
  CHECK_MSG(img.GetWidth() == 0, "width should be 0");
  CHECK_MSG(img.GetHeight() == 0, "height should be 0");
  PASS();
}

// ========================================================
// Test 8: Resize (no aspect ratio)
// ========================================================
void test_resize_no_aspect_ratio() {
  TEST("Resize without maintain aspect ratio");
  WebImage img("img.png");

  img.Resize(400, 300, false);
  REQUIRE_MSG(img.GetWidth() == 400, "width mismatch");
  REQUIRE_MSG(img.GetHeight() == 300, "height mismatch");
  PASS();
}

// ========================================================
// Test 9: Resize (maintain aspect ratio)
// ========================================================
void test_resize_maintain_aspect_ratio() {
  TEST("Resize with maintain aspect ratio");
  WebImage img("img.png");

  img.Resize(800, 600, true);
  REQUIRE_MSG(img.GetWidth() == 800, "width mismatch");
  REQUIRE_MSG(img.GetHeight() == 600, "height mismatch");
  PASS();
}

// ========================================================
// Test 10: Resize default parameter (maintain_aspect_ratio = false)
// ========================================================
void test_resize_default_param() {
  TEST("Resize default maintain_aspect_ratio is false");
  WebImage img("img.png");

  img.Resize(500, 400);  // uses default param
  CHECK_MSG(img.GetWidth() == 500, "width mismatch");
  CHECK_MSG(img.GetHeight() == 400, "height mismatch");
  PASS();
}

// ========================================================
// Test 11: SetOpacity / GetOpacity
// ========================================================
void test_set_get_opacity() {
  TEST("SetOpacity / GetOpacity");
  WebImage img("img.png");

  REQUIRE_MSG(img.GetOpacity() == 1.0, "default opacity should be 1.0");
  img.SetOpacity(0.5);
  CHECK_MSG(img.GetOpacity() == 0.5, "opacity not updated to 0.5");
  img.SetOpacity(0.0);
  CHECK_MSG(img.GetOpacity() == 0.0, "opacity not updated to 0.0");
  img.SetOpacity(1.0);
  CHECK_MSG(img.GetOpacity() == 1.0, "opacity not updated to 1.0");
  PASS();
}

// ========================================================
// Test 12: Show / Hide / IsVisible
// ========================================================
void test_show_hide_visibility() {
  TEST("Show / Hide / IsVisible");
  WebImage img("img.png");

  REQUIRE_MSG(img.IsVisible() == true, "default should be visible");
  img.Hide();
  CHECK_MSG(img.IsVisible() == false, "should be hidden after Hide()");
  img.Show();
  CHECK_MSG(img.IsVisible() == true, "should be visible after Show()");
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
    CHECK_MSG(img.IsVisible() == false, "should be hidden");
    img.Show();
    CHECK_MSG(img.IsVisible() == true, "should be visible");
  }
  PASS();
}

// ========================================================
// Test 14: Loading state via HandleLoad and SetSource
// ========================================================
void test_loaded_state() {
  TEST("Loading state via HandleLoad and SetSource");
  WebImage img("img.png");

  REQUIRE_MSG(img.IsLoaded() == false, "default should not be loaded");
  img.HandleLoad();
  CHECK_MSG(img.IsLoaded() == true, "should be loaded after HandleLoad");
  img.SetSource("other.png");
  CHECK_MSG(img.IsLoaded() == false, "should not be loaded after SetSource");
  PASS();
}

// ========================================================
// Test 15: HasError — default
// ========================================================
void test_has_error_default() {
  TEST("HasError default is false");
  WebImage img("img.png");

  REQUIRE_MSG(img.HasError() == false, "default should have no error");
  PASS();
}

// ========================================================
// Test 16: HandleLoad
// ========================================================
void test_handle_load() {
  TEST("HandleLoad sets loaded=true, error=false");
  WebImage img("img.png");

  img.HandleLoad();
  REQUIRE_MSG(img.IsLoaded() == true, "should be loaded after HandleLoad");
  CHECK_MSG(img.HasError() == false, "should have no error after HandleLoad");
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
  REQUIRE_MSG(called == true, "on_load_callback should have been called");
  PASS();
}

// ========================================================
// Test 18: HandleError
// ========================================================
void test_handle_error() {
  TEST("HandleError sets error=true, loaded=false");
  WebImage img("img.png");

  img.HandleError();
  REQUIRE_MSG(img.HasError() == true, "should have error after HandleError");
  CHECK_MSG(img.IsLoaded() == false, "should not be loaded after HandleError");
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
  REQUIRE_MSG(called == true, "on_error_callback should have been called");
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
  REQUIRE_MSG(img.HasError() == true, "should have error");
  CHECK_MSG(error_called == true, "error callback should be called");
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
  REQUIRE_MSG(img.HasError() == true, "should have error");
  CHECK_MSG(error_called == true, "error callback should be called even in NoOp");
  PASS();
}

// ========================================================
// Test 22: SetErrorMode
// ========================================================
void test_set_error_mode() {
  TEST("SetErrorMode changes mode");
  WebImage img("img.png");

  img.SetErrorMode(ImageErrorMode::NoOp);
  img.HandleError();
  CHECK_MSG(img.HasError() == true, "should have error");

  img.SetSource("img2.png");
  img.SetErrorMode(ImageErrorMode::BlankRect);
  img.HandleError();
  CHECK_MSG(img.HasError() == true, "should have error with BlankRect");
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

  REQUIRE_MSG(img1.Id() != img2.Id(), "img1 and img2 should have different ids");
  REQUIRE_MSG(img2.Id() != img3.Id(), "img2 and img3 should have different ids");
  REQUIRE_MSG(img1.Id() != img3.Id(), "img1 and img3 should have different ids");
  PASS();
}

// ========================================================
// Test 25: Id() starts with expected prefix
// ========================================================
void test_id_prefix() {
  TEST("Id() starts with 'webimage-'");
  WebImage img("x.png");

  std::string id = img.Id();
  REQUIRE_MSG(id.substr(0, 9) == "webimage-", "id should start with 'webimage-'");
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
  img.SyncFromModel();

  REQUIRE_MSG(img.GetSource() == "img.png", "source should remain");
  CHECK_MSG(img.GetAltText() == "alt", "alt text should remain");
  CHECK_MSG(img.GetWidth() == 300, "width should remain");
  CHECK_MSG(img.GetHeight() == 200, "height should remain");
  CHECK_MSG(img.GetOpacity() == 0.8, "opacity should remain 0.8");
  CHECK_MSG(img.IsVisible() == false, "should remain hidden");
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
  original.HandleLoad();
  std::string original_id = original.Id();

  WebImage moved(std::move(original));

  REQUIRE_MSG(moved.GetSource() == "move.png", "source should transfer");
  CHECK_MSG(moved.GetAltText() == "moving", "alt text should transfer");
  CHECK_MSG(moved.GetWidth() == 100, "width should transfer");
  CHECK_MSG(moved.GetHeight() == 50, "height should transfer");
  CHECK_MSG(moved.GetOpacity() == 0.7, "opacity should transfer");
  CHECK_MSG(moved.IsVisible() == false, "visibility should transfer");
  CHECK_MSG(moved.IsLoaded() == true, "loaded state should transfer");
  CHECK_MSG(moved.Id() == original_id, "id should transfer");

  CHECK_MSG(original.GetWidth() == 0, "original width should be 0");
  CHECK_MSG(original.GetHeight() == 0, "original height should be 0");
  CHECK_MSG(original.GetOpacity() == 1.0, "original opacity should reset to 1.0");
  CHECK_MSG(original.IsVisible() == false, "original visibility should be false");
  CHECK_MSG(original.IsLoaded() == false, "original loaded should be false");
  CHECK_MSG(original.HasError() == false, "original error should be false");
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

  REQUIRE_MSG(dest.GetSource() == "src.png", "source should transfer");
  CHECK_MSG(dest.GetAltText() == "source img", "alt text should transfer");
  CHECK_MSG(dest.GetWidth() == 64, "width should transfer");
  CHECK_MSG(dest.GetHeight() == 64, "height should transfer");
  CHECK_MSG(dest.GetOpacity() == 0.3, "opacity should transfer");
  CHECK_MSG(dest.Id() == src_id, "id should transfer");

  CHECK_MSG(src.GetWidth() == 0, "src width should be 0");
  CHECK_MSG(src.GetHeight() == 0, "src height should be 0");
  CHECK_MSG(src.IsLoaded() == false, "src loaded should be false");
  PASS();
}

// ========================================================
// Test 29: HandleLoad after HandleError resets error
// ========================================================
void test_load_after_error() {
  TEST("HandleLoad after HandleError resets error state");
  WebImage img("img.png");

  img.HandleError();
  REQUIRE_MSG(img.HasError() == true, "should have error");
  CHECK_MSG(img.IsLoaded() == false, "should not be loaded");

  img.HandleLoad();
  CHECK_MSG(img.HasError() == false, "error should be cleared after load");
  CHECK_MSG(img.IsLoaded() == true, "should be loaded");
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
  REQUIRE_MSG(first_call_count == 1, "first callback should be called once");

  img.SetOnLoadCallback([&second_call_count]() { ++second_call_count; });
  img.HandleLoad();
  CHECK_MSG(first_call_count == 1, "first callback should still be 1");
  CHECK_MSG(second_call_count == 1, "second callback should be called once");
  PASS();
}

// ========================================================
// Test 31: No callback set — HandleLoad/HandleError should not crash
// ========================================================
void test_no_callback_no_crash() {
  TEST("HandleLoad/HandleError without callbacks does not crash");
  WebImage img("img.png");

  img.HandleLoad();
  img.HandleError();
  REQUIRE_MSG(img.HasError() == true, "should have error");
  PASS();
}

// ========================================================
// Test 32: SetSize then Resize overwrites
// ========================================================
void test_set_size_then_resize() {
  TEST("Resize after SetSize overwrites dimensions");
  WebImage img("img.png");

  img.SetSize(100, 100);
  REQUIRE_MSG(img.GetWidth() == 100, "width should be 100");
  REQUIRE_MSG(img.GetHeight() == 100, "height should be 100");

  img.Resize(200, 150, true);
  CHECK_MSG(img.GetWidth() == 200, "width should be 200 after Resize");
  CHECK_MSG(img.GetHeight() == 150, "height should be 150 after Resize");
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
    callback_saw_error = img.HasError();
  });

  img.HandleError();
  REQUIRE_MSG(callback_saw_error == true,
        "callback should see has_error=true");
  PASS();
}

// ========================================================
// Test 34: Draw() does not crash (stub)
// ========================================================
void test_draw_no_crash() {
  TEST("Draw() stub does not crash");
  WebImage img("img.png");

  // Draw() is a stub — a real test would need a mock WebCanvas.
  PASS();
}

// ========================================================
// Test 35: Multiple source changes
// ========================================================
void test_multiple_source_changes() {
  TEST("Multiple SetSource calls update correctly");
  WebImage img("first.png");

  img.SetSource("second.png");
  REQUIRE_MSG(img.GetSource() == "second.png", "source should be second.png");

  img.SetSource("third.png");
  CHECK_MSG(img.GetSource() == "third.png", "source should be third.png");

  img.SetSource("");
  CHECK_MSG(img.GetSource().empty(), "source should be empty");
  PASS();
}

// ==================== TEST_CASE Registration ====================

// -- Construction --
TEST_CASE("Constructor sets source and alt text", "[WebImage]") { test_constructor(); }
TEST_CASE("Constructor default alt text is empty", "[WebImage]") { test_constructor_default_alt(); }

// -- Source & metadata setters/getters --
TEST_CASE("SetSource and GetSource", "[WebImage]") { test_set_get_source(); }
TEST_CASE("SetSource resets loaded and error state", "[WebImage]") { test_set_source_resets_state(); }
TEST_CASE("SetAltText and GetAltText", "[WebImage]") { test_set_get_alt_text(); }

// -- Sizing (SetSize and Resize with various dimensions) --
TEST_CASE("SetSize sets width and height", "[WebImage]") { test_set_size(); }
TEST_CASE("SetSize handles zero dimensions", "[WebImage]") { test_set_size_zero(); }
TEST_CASE("Resize without aspect ratio", "[WebImage]") { test_resize_no_aspect_ratio(); }
TEST_CASE("Resize with aspect ratio", "[WebImage]") { test_resize_maintain_aspect_ratio(); }
TEST_CASE("Resize default parameter", "[WebImage]") { test_resize_default_param(); }

// -- Opacity --
TEST_CASE("SetOpacity and GetOpacity", "[WebImage]") { test_set_get_opacity(); }

// -- Visibility (Show/Hide toggle) --
TEST_CASE("Show Hide visibility", "[WebImage]") { test_show_hide_visibility(); }
TEST_CASE("Visibility toggle multiple times", "[WebImage]") { test_visibility_toggle_multiple(); }

// -- Loading state (tracked via HandleLoad / SetSource; MarkLoaded is private) --
TEST_CASE("Loading state via HandleLoad and SetSource", "[WebImage]") { test_loaded_state(); }
TEST_CASE("HasError default false", "[WebImage]") { test_has_error_default(); }

// -- Load/error event handlers and callbacks --
TEST_CASE("HandleLoad sets state", "[WebImage]") { test_handle_load(); }
TEST_CASE("HandleLoad invokes callback", "[WebImage]") { test_handle_load_callback(); }
TEST_CASE("HandleError sets state", "[WebImage]") { test_handle_error(); }
TEST_CASE("HandleError invokes callback", "[WebImage]") { test_handle_error_callback(); }
TEST_CASE("HandleError with BlankRect mode", "[WebImage]") { test_handle_error_blank_rect(); }
TEST_CASE("HandleError with NoOp mode", "[WebImage]") { test_handle_error_noop(); }

// -- Error mode and placeholder configuration --
TEST_CASE("SetErrorMode works", "[WebImage]") { test_set_error_mode(); }
TEST_CASE("SetPlaceholderColor does not crash", "[WebImage]") { test_set_placeholder_color(); }

// -- Unique element IDs --
TEST_CASE("Id is unique per instance", "[WebImage]") { test_id_unique(); }
TEST_CASE("Id has expected prefix", "[WebImage]") { test_id_prefix(); }

// -- DOM synchronization --
TEST_CASE("syncFromModel reapplies state", "[WebImage]") { test_sync_from_model(); }

// -- Move semantics (constructor and assignment) --
TEST_CASE("Move constructor transfers state", "[WebImage]") { test_move_constructor(); }
TEST_CASE("Move assignment transfers state", "[WebImage]") { test_move_assignment(); }

// -- Edge cases and regression scenarios --
TEST_CASE("HandleLoad after error clears error", "[WebImage]") { test_load_after_error(); }
TEST_CASE("Callback replacement works", "[WebImage]") { test_callback_replacement(); }
TEST_CASE("No callback no crash", "[WebImage]") { test_no_callback_no_crash(); }
TEST_CASE("Resize after SetSize overwrites dimensions", "[WebImage]") { test_set_size_then_resize(); }
TEST_CASE("Error callback with placeholder", "[WebImage]") { test_error_callback_with_placeholder(); }
TEST_CASE("draw stub no crash", "[WebImage]") { test_draw_no_crash(); }
TEST_CASE("Multiple SetSource calls", "[WebImage]") { test_multiple_source_changes(); }

#endif // __EMSCRIPTEN__
