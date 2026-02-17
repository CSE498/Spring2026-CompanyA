#include "../../../source/Interfaces/WebUI/WebTextbox/WebTextbox.hpp"
#include "../../../source/Interfaces/WebUI/WebLayout/WebLayout.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <emscripten.h>
#include <emscripten/val.h>

using emscripten::val;

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

static val GetDocument() 
{ 
    return val::global("document"); 
}
static val GetWindow()   
{ 
    return val::global("window"); 
}

static val ById(const std::string& id) 
{
  return GetDocument().call<val>("getElementById", id);
}

static std::string GetComputedStyleStr(val element, const std::string& prop) 
{
  val win = GetWindow();
  val style = win.call<val>("getComputedStyle", element);
  return style.call<val>("getPropertyValue", prop).as<std::string>();
}

static double GetBoundingClientRectNumber(val element, const std::string& key) 
{
  val rect = element.call<val>("getBoundingClientRect");
  return rect[key].as<double>();
}

// Make a unique root id for layouts in tests so tests dont interfere with each other in the DOM
static std::string UniqueRootId(const std::string& prefix) 
{
  static int counter = 1;
  std::ostringstream ss;
  ss << prefix << "-" << counter++;
  return ss.str();
}

// ========================================================
// Test 1: Constructor — defaults set correctly
// ========================================================
void test_constructor() 
{
  TEST("Constructor sets defaults correctly");
  WebTextbox tb("hello");

  CHECK(tb.GetText() == "hello", "initial text mismatch");
  CHECK(tb.IsVisible() == true, "default should be visible");
  CHECK(!tb.Id().empty(), "id should not be empty");
  PASS();
}

// ========================================================
// Test 2: Constructor — default text empty
// ========================================================
void test_constructor_default_text() 
{
  TEST("Constructor default text is empty");
  WebTextbox tb;

  CHECK(tb.GetText().empty(), "default text should be empty");
  PASS();
}

// ========================================================
// Test 3: SetText / GetText
// ========================================================
void test_set_get_text() 
{
  TEST("SetText / GetText");
  WebTextbox tb("old");

  tb.SetText("new");
  CHECK(tb.GetText() == "new", "text not updated");
  PASS();
}

// ========================================================
// Test 4: AppendText
// ========================================================
void test_append_text() 
{
  TEST("AppendText appends correctly");
  WebTextbox tb("Hello");

  tb.AppendText(", world!");
  CHECK(tb.GetText() == "Hello, world!", "append failed");
  PASS();
}

// ========================================================
// Test 5: Clear
// ========================================================
void test_clear() 
{
  TEST("Clear empties text");
  WebTextbox tb("something");

  tb.Clear();
  CHECK(tb.GetText().empty(), "text should be empty after Clear()");
  PASS();
}

// ========================================================
// Test 6: Show / Hide / IsVisible
// ========================================================
void test_show_hide_visibility() 
{
  TEST("Show / Hide / IsVisible");
  WebTextbox tb("x");

  CHECK(tb.IsVisible() == true, "default should be visible");
  tb.Hide();
  CHECK(tb.IsVisible() == false, "should be hidden after Hide()");
  tb.Show();
  CHECK(tb.IsVisible() == true, "should be visible after Show()");
  PASS();
}

// ========================================================
// Test 7: Multiple Show/Hide toggles
// ========================================================
void test_visibility_toggle_multiple() 
{
  TEST("Multiple Show/Hide toggles");
  WebTextbox tb("x");

  for (int i = 0; i < 5; ++i) 
  {
    tb.Hide();
    CHECK(tb.IsVisible() == false, "should be hidden");
    tb.Show();
    CHECK(tb.IsVisible() == true, "should be visible");
  }
  PASS();
}

// ========================================================
// Test 8: syncFromModel (no crash, state preserved)
// ========================================================
void test_sync_from_model() 
{
  TEST("syncFromModel does not crash and preserves state");
  WebTextbox tb("hi");

  tb.SetFontFamily("Arial");
  tb.SetFontSize(18.0f);
  tb.SetBold(true);
  tb.SetItalic(true);
  tb.SetColor("#FF0000");
  tb.SetAlignment("center");
  tb.SetMaxWidth(300.0f);
  tb.SetWrap(true);
  tb.Hide();

  tb.syncFromModel();

  CHECK(tb.GetText() == "hi", "text should remain");
  CHECK(tb.IsVisible() == false, "visibility should remain hidden");
  PASS();
}

// ========================================================
// Test 9: Bounding box returns 0 when not mounted
// ========================================================
void test_bounding_box_not_mounted() 
{
  TEST("Bounding box is zero when not mounted");
  WebTextbox tb("measure me");

  auto r = tb.GetBoundingBoxPx();
  CHECK(r.w == 0.0, "width should be 0 when not mounted");
  CHECK(r.h == 0.0, "height should be 0 when not mounted");
  PASS();
}

// ========================================================
// Test 10: Id() uniqueness
// ========================================================
void test_id_unique() 
{
  TEST("Id() is unique per instance");
  WebTextbox a("a");
  WebTextbox b("b");
  WebTextbox c("c");

  CHECK(a.Id() != b.Id(), "a and b should have different ids");
  CHECK(b.Id() != c.Id(), "b and c should have different ids");
  CHECK(a.Id() != c.Id(), "a and c should have different ids");
  PASS();
}

// ========================================================
// Test 11: Id() prefix
// ========================================================
void test_id_prefix() 
{
  TEST("Id() starts with expected prefix");
  WebTextbox tb("x");

  std::string id = tb.Id();
  // Adjust this prefix if your implementation uses something else.
  CHECK(id.rfind("webtextbox-", 0) == 0, "id should start with 'webtextbox-'");
  PASS();
}

// ========================================================
// Test 12: Move constructor transfers state
// ========================================================
void test_move_constructor() 
{
  TEST("Move constructor transfers state");
  WebTextbox original("move me");
  original.SetFontFamily("Arial");
  original.SetFontSize(22.0f);
  original.SetBold(true);
  original.Hide();
  std::string original_id = original.Id();

  WebTextbox moved(std::move(original));

  CHECK(moved.GetText() == "move me", "text should transfer");
  CHECK(moved.IsVisible() == false, "visibility should transfer");
  CHECK(moved.Id() == original_id, "id should transfer");

  // Moved-from: visibility typically false in your patterns
  CHECK(original.IsVisible() == false, "original should be hidden after move");
  PASS();
}

// ========================================================
// Test 13: Move assignment transfers state
// ========================================================
void test_move_assignment() 
{
  TEST("Move assignment transfers state");
  WebTextbox src("source");
  src.SetBold(true);
  src.Hide();
  std::string src_id = src.Id();

  WebTextbox dest("dest");
  dest = std::move(src);

  CHECK(dest.GetText() == "source", "text should transfer");
  CHECK(dest.IsVisible() == false, "visibility should transfer");
  CHECK(dest.Id() == src_id, "id should transfer");
  PASS();
}

// ========================================================
// Test 14: Alignment accepts valid values
// ========================================================
void test_alignment_valid_values_no_crash() 
{
  TEST("SetAlignment accepts left/center/right");
  WebTextbox tb("x");

  tb.SetAlignment("left");
  tb.SetAlignment("center");
  tb.SetAlignment("right");
  PASS();
}

// ========================================================
// Test 15: Background color set/clear
// ========================================================
void test_background_color_no_crash() 
{
  TEST("SetBackgroundColor / ClearBackgroundColor does not crash");
  WebTextbox tb("x");

  tb.SetBackgroundColor("#AABBCC");
  tb.ClearBackgroundColor();
  tb.SetBackgroundColor("transparent");
  PASS();
}

// ========================================================
// Test 16: Font fallback
// ========================================================
void test_font_fallback_no_crash() 
{
  TEST("SetFallbackFontFamily does not crash");
  WebTextbox tb("x");

  tb.SetFontFamily("DefinitelyNotARealFont");
  tb.SetFallbackFontFamily("serif");
  PASS();
}

// ========================================================
// Test 17: MaxWidth / Wrap interaction
// ========================================================
void test_maxwidth_wrap_no_crash() 
{
  TEST("SetMaxWidth / SetWrap does not crash");
  WebTextbox tb("long long long long long");

  tb.SetMaxWidth(200.0f);
  tb.SetWrap(true);
  tb.SetWrap(false);
  PASS();
}

// ========================================================
// Test 18: mountToLayout creates DOM element
// ========================================================
void test_mount_creates_dom_element() 
{
  TEST("mountToLayout makes element appear in DOM");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.SetSpacing(5);
  root.Apply();

  WebTextbox tb("hello");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  val el = ById(tb.Id());
  CHECK(!el.isNull() && !el.isUndefined(), "element should exist in DOM after mount");
  PASS();
}

// ========================================================
// Test 19: unmount removes DOM element
// ========================================================
void test_unmount_removes_dom_element() 
{
  TEST("unmount removes element from DOM");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("bye");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  tb.unmount();
  val el = ById(tb.Id());
  CHECK(el.isNull() || el.isUndefined(), "element should not exist after unmount");
  PASS();
}

// ========================================================
// Test 20: SetText updates DOM textContent when mounted
// ========================================================
void test_settext_updates_dom_textContent() 
{
  TEST("SetText updates DOM textContent");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("old");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  tb.SetText("new text");
  val el = ById(tb.Id());
  CHECK(el["textContent"].as<std::string>() == "new text", "textContent mismatch after SetText");
  PASS();
}

// ========================================================
// Test 21: AppendText updates DOM textContent when mounted
// ========================================================
void test_append_updates_dom_textContent() 
{
  TEST("AppendText updates DOM textContent");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("A");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  tb.AppendText("B");
  val el = ById(tb.Id());
  CHECK(el["textContent"].as<std::string>() == "AB", "textContent mismatch after AppendText");
  PASS();
}

// ========================================================
// Test 22: Show/Hide affects computed display when mounted
// ========================================================
void test_show_hide_affects_display_style() 
{
  TEST("Show/Hide affects computed display");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("x");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  tb.Hide();
  val el = ById(tb.Id());
  std::string disp1 = GetComputedStyleStr(el, "display");
  CHECK(disp1 == "none", "display should be none after Hide()");

  tb.Show();
  std::string disp2 = GetComputedStyleStr(el, "display");
  CHECK(disp2 != "none", "display should not be none after Show()");
  PASS();
}

// ========================================================
// Test 23: SetFontSize affects computed font-size
// ========================================================
void test_font_size_computed() 
{
  TEST("SetFontSize changes computed font-size");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("size");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  tb.SetFontSize(24.0f);
  val el = ById(tb.Id());
  std::string fs = GetComputedStyleStr(el, "font-size");
  CHECK(fs.find("24") != std::string::npos, "font-size should contain 24px");
  PASS();
}

// ========================================================
// Test 24: Bold/Italic affect computed styles
// ========================================================
void test_bold_italic_computed() 
{
  TEST("SetBold/SetItalic changes computed font-weight/style");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("style");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  tb.SetBold(true);
  tb.SetItalic(true);

  val el = ById(tb.Id());
  std::string fw = GetComputedStyleStr(el, "font-weight");
  std::string fs = GetComputedStyleStr(el, "font-style");

  // font-weight is often "700" for bold
  CHECK((fw == "bold" || fw == "700" || fw == "800" || fw == "900"), "font-weight should be bold-ish");
  CHECK(fs == "italic", "font-style should be italic");
  PASS();
}

// ========================================================
// Test 25: SetColor affects computed color
// ========================================================
void test_color_computed() 
{
  TEST("SetColor changes computed color");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("color");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  tb.SetColor("rgb(255, 0, 0)");
  val el = ById(tb.Id());
  std::string c = GetComputedStyleStr(el, "color");

  // browsers normalize to rgb(...) strings
  CHECK(c.find("255") != std::string::npos, "color should contain 255");
  PASS();
}

// ========================================================
// Test 26: SetAlignment affects computed text-align
// ========================================================
void test_alignment_computed() 
{
  TEST("SetAlignment changes computed text-align");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("align");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  tb.SetAlignment("center");
  val el = ById(tb.Id());
  std::string ta = GetComputedStyleStr(el, "text-align");
  CHECK(ta == "center", "text-align should be center");
  PASS();
}

// ========================================================
// Test 27: SetMaxWidth affects computed max-width
// ========================================================
void test_max_width_computed() 
{
  TEST("SetMaxWidth changes computed max-width");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("max width");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  tb.SetMaxWidth(123.0f);
  val el = ById(tb.Id());
  std::string mw = GetComputedStyleStr(el, "max-width");

  CHECK(mw.find("123") != std::string::npos, "max-width should contain 123px");
  PASS();
}

// ========================================================
// Test 28: Wrap toggles white-space computed property
// ========================================================
void test_wrap_changes_white_space() 
{
  TEST("SetWrap toggles computed white-space");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("wrap");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  tb.SetWrap(true);
  val el = ById(tb.Id());
  std::string ws1 = GetComputedStyleStr(el, "white-space");
  CHECK(ws1.find("pre-wrap") != std::string::npos || ws1 == "pre-wrap", "white-space should be pre-wrap when wrap enabled");

  tb.SetWrap(false);
  std::string ws2 = GetComputedStyleStr(el, "white-space");
  CHECK(ws2.find("pre") != std::string::npos, "white-space should be pre when wrap disabled");
  PASS();
}

// ========================================================
// Test 29: Background color applies via computed background-color
// ========================================================
void test_background_color_computed() 
{
  TEST("Background color applies via computed background-color");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("bg");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  tb.SetBackgroundColor("rgb(1, 2, 3)");
  val el = ById(tb.Id());
  std::string bg = GetComputedStyleStr(el, "background-color");
  CHECK(bg.find("1") != std::string::npos, "background-color should contain 1");
  PASS();
}

// ========================================================
// Test 30: Bounding box becomes non-zero when mounted
// ========================================================
void test_bounding_box_after_mount_nonzero() 
{
  TEST("Bounding box is non-zero after mount");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("Hello bounding box");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  auto r = tb.GetBoundingBoxPx();
  CHECK(r.w > 0.0, "width should be > 0 after mount");
  CHECK(r.h > 0.0, "height should be > 0 after mount");
  PASS();
}

// ========================================================
// Test 31: Multi-line text increases height vs single line
// ========================================================
void test_multiline_increases_height() 
{
  TEST("Multi-line text increases height");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("Line1");
  tb.SetFontSize(20.0f);
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  double h1 = tb.GetHeightPx();
  tb.SetText("Line1\nLine2\nLine3");
  root.Apply();

  double h2 = tb.GetHeightPx();
  CHECK(h2 > h1, "height should increase with multiple lines");
  PASS();
}

// ========================================================
// Test 32: Layout alignment sets align-self on the child
// ========================================================
void test_layout_alignment_sets_alignself() 
{
  TEST("Alignment sets align-self style");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("align-self");
  tb.mountToLayout(root, Alignment::End);
  root.Apply();

  val el = ById(tb.Id());
  std::string as = GetComputedStyleStr(el, "align-self");

  // flex-end is expected for Alignment::End
  CHECK(as == "flex-end" || as == "end", "align-self should be flex-end-ish");
  PASS();
}

// ========================================================
// Test 33: syncFromModel repairs DOM if style is manually changed
// ========================================================
void test_sync_repairs_style_mutation() 
{
  TEST("syncFromModel repairs manual DOM style mutation");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("repair");
  tb.SetFontSize(18.0f);
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  val el = ById(tb.Id());
  // Mutate DOM directly
  el["style"].set("fontSize", std::string("2px"));
  std::string fs_bad = GetComputedStyleStr(el, "font-size");
  CHECK(fs_bad.find("2") != std::string::npos, "font-size should be 2px after mutation");

  // Repair from model
  tb.syncFromModel();
  std::string fs_fixed = GetComputedStyleStr(el, "font-size");
  CHECK(fs_fixed.find("18") != std::string::npos, "font-size should be restored to ~18px");
  PASS();
}

// ========================================================
// Test 34: Move + mount keeps element accessible by same id
// ========================================================
void test_move_then_mount_element_exists() 
{
  TEST("Move then mount keeps element accessible");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox a("move-dom");
  std::string id = a.Id();

  WebTextbox b(std::move(a));
  b.mountToLayout(root, Alignment::Start);
  root.Apply();

  val el = ById(id);
  CHECK(!el.isNull() && !el.isUndefined(), "moved element should exist by same id after mount");
  CHECK(el["textContent"].as<std::string>() == "move-dom", "textContent should remain after move");
  PASS();
}

// ========================================================
// Test 35: Two textboxes mount and preserve ordering in layout
// ========================================================
void test_layout_ordering() 
{
  TEST("Layout preserves order of mounted textboxes");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox a("A");
  WebTextbox b("B");
  a.mountToLayout(root, Alignment::Start);
  b.mountToLayout(root, Alignment::Start);
  root.Apply();

  val rootEl = ById(root.Id());
  val c0 = rootEl["children"][0];
  val c1 = rootEl["children"][1];

  CHECK(c0["id"].as<std::string>() == a.Id(), "first child should be textbox A");
  CHECK(c1["id"].as<std::string>() == b.Id(), "second child should be textbox B");
  PASS();
}

// ========================================================
// Test 36: "Coordination with WebButton"
// Put textbox inside a <button> and verify containment.
// ========================================================
void test_textbox_can_live_inside_button_dom() 
{
  TEST("Textbox can be embedded inside an HTML button (coordination demo)");
  val doc = GetDocument();

  // Layout root
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  // Make a real button in DOM
  std::string btn_id = UniqueRootId("btn");
  val btn = doc.call<val>("createElement", std::string("button"));
  btn.set("id", btn_id);
  btn.set("textContent", std::string("Button: "));
  doc["body"].call<void>("appendChild", btn);

  // Mount textbox, then move its DOM node into the button
  WebTextbox tb("Label");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  val tbEl = ById(tb.Id());
  CHECK(!tbEl.isNull() && !tbEl.isUndefined(), "textbox element should exist");

  // Move textbox inside the button
  btn.call<void>("appendChild", tbEl);

  // Verify containment
  bool contains = btn.call<val>("contains", tbEl).as<bool>();
  CHECK(contains == true, "button should contain textbox element");

  // Verify combined visible text (button textContent should include Label somewhere)
  std::string combined = btn["textContent"].as<std::string>();
  CHECK(combined.find("Label") != std::string::npos, "button textContent should include textbox text");

  // cleanup: remove button
  doc["body"].call<void>("removeChild", btn);

  PASS();
}

// ========================================================
// Test 37: Clearing background returns transparent-ish
// ========================================================
void test_clear_background_transparent() 
{
  TEST("ClearBackgroundColor results in transparent background");
  WebLayout root(UniqueRootId("layout"));
  root.SetLayoutType(LayoutType::Vertical);
  root.Apply();

  WebTextbox tb("bg-clear");
  tb.mountToLayout(root, Alignment::Start);
  root.Apply();

  tb.SetBackgroundColor("rgb(9, 9, 9)");
  tb.ClearBackgroundColor();

  val el = ById(tb.Id());
  std::string bg = GetComputedStyleStr(el, "background-color");

  CHECK(bg.find("rgba") != std::string::npos || bg == "transparent", "background-color should be transparent-ish after clear");
  PASS();
}

// ========================================================
// Main — run all tests
// ========================================================
int main() 
{
  std::cout << "======================================" << std::endl;
  std::cout << "  WebTextbox Unit Tests" << std::endl;
  std::cout << "======================================" << std::endl;

  test_constructor();
  test_constructor_default_text();
  test_set_get_text();
  test_append_text();
  test_clear();
  test_show_hide_visibility();
  test_visibility_toggle_multiple();
  test_sync_from_model();
  test_bounding_box_not_mounted();
  test_id_unique();
  test_id_prefix();
  test_move_constructor();
  test_move_assignment();
  test_alignment_valid_values_no_crash();
  test_background_color_no_crash();
  test_font_fallback_no_crash();
  test_maxwidth_wrap_no_crash();
  test_mount_creates_dom_element();
  test_unmount_removes_dom_element();
  test_settext_updates_dom_textContent();
  test_append_updates_dom_textContent();
  test_show_hide_affects_display_style();
  test_font_size_computed();
  test_bold_italic_computed();
  test_color_computed();
  test_alignment_computed();
  test_max_width_computed();
  test_wrap_changes_white_space();
  test_background_color_computed();
  test_bounding_box_after_mount_nonzero();
  test_multiline_increases_height();
  test_layout_alignment_sets_alignself();
  test_sync_repairs_style_mutation();
  test_move_then_mount_element_exists();
  test_layout_ordering();
  test_textbox_can_live_inside_button_dom();
  test_clear_background_transparent();

  std::cout << "======================================" << std::endl;
  std::cout << "  Results: " << tests_passed << " passed, "
            << tests_failed << " failed, "
            << (tests_passed + tests_failed) << " total" << std::endl;
  std::cout << "======================================" << std::endl;

  if (tests_failed > 0) 
  {
    std::cout << "  *** SOME TESTS FAILED ***" << std::endl;
    return 1;
  } 
  else 
  {
    std::cout << "  All tests passed!" << std::endl;
    return 0;
  }
}