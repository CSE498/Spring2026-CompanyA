/**
 * WebTextboxTest.cpp
 * Unit tests for the WebTextbox class using the Catch2 testing framework.
 *
 * These tests verify behavior including text updates, styling, visibility,
 * layout integration, DOM synchronization, bounding boxes, and move semantics
 * when compiled to WebAssembly using Emscripten.
 * 
 * Setup Instructions:
 *      1. Ensure Emscripten SDK is installed and activated.
 *      2. Go to cd emsdk directory and run `emsdk install latest` and `emsdk activate latest`.
 *      3. Run emsdk_env.bat to set up environment variables.
 *      4. cd "..\CSE 498\CompanyA\Spring2026-CompanyA\tests\Interfaces\WebUI"
 *      5. Compile with:`em++ WebTextboxTest.cpp ../../../source/Interfaces/WebUI/WebTextbox/WebTextbox.cpp ../../../source/Interfaces/WebUI/WebLayout/WebLayout.cpp -I../../../ -I../../../third-party/Catch/single_include --bind -std=c++23 -s WASM=1 -s ASSERTIONS=1 -o test.html`
 *      6. Run python -m http.server 8000 for local hosting 
 *      7. Open test.html in a browser to run the tests and view results in the console (http://localhost:8000/test.html)
 * 
 * Additional Note:
 *      Portions of formatting, documentation, and cleanup were assisted by
 *      AI tooling to improve consistency and readability.
 *
 * All project classes and tests correspond to the cse498 WebUI subsystem.
 *
 * author Lance Motowski
 * date   Spring 2026
 */

#ifdef __EMSCRIPTEN__

// Generate Catch2 main
#define CATCH_CONFIG_MAIN
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../../source/Interfaces/WebUI/WebTextbox/WebTextbox.hpp"
#include "../../../source/Interfaces/WebUI/WebLayout/WebLayout.hpp"

#include <string>
#include <sstream>
#include <emscripten/val.h>

using cse498::WebTextbox;
using cse498::WebLayout;
using cse498::Alignment;
using cse498::LayoutType;

using emscripten::val;

// ---------- Helpers ----------

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

// static double GetBoundingClientRectNumber(val element, const std::string& key)
// {
//     val rect = element.call<val>("getBoundingClientRect");
//     return rect[key].as<double>();
// }

// Make a unique root id for layouts in tests so tests dont interfere with each other in the DOM
static std::string UniqueRootId(const std::string& prefix)
{
    static int counter = 1;
    std::ostringstream ss;
    ss << prefix << "-" << counter++;
    return ss.str();
}

// ---------- Map CHECK macro onto Catch2 ----------
// Already had a CHECK macro for internal use, map it to Catch2 REQUIRE with message support for better test output.

#undef CHECK
#define CHECK(cond, msg)          \
    do {                          \
        INFO(msg);                \
        REQUIRE((cond));            \
    } while (0)

// ========================================================
// Test 1: Constructor — defaults set correctly
// ========================================================
TEST_CASE("Constructor sets defaults correctly", "[webtextbox]")
{
    WebTextbox tb("hello");

    CHECK(tb.GetText() == "hello", "initial text mismatch");
    CHECK(tb.IsVisible() == true, "default should be visible");
    CHECK(!tb.Id().empty(), "id should not be empty");
}

// ========================================================
// Test 2: Constructor — default text empty
// ========================================================
TEST_CASE("Constructor default text is empty", "[webtextbox]")
{
    WebTextbox tb;

    CHECK(tb.GetText().empty(), "default text should be empty");
}

// ========================================================
// Test 3: SetText / GetText
// ========================================================
TEST_CASE("SetText / GetText", "[webtextbox]")
{
    WebTextbox tb("old");

    tb.SetText("new");
    CHECK(tb.GetText() == "new", "text not updated");
}

// ========================================================
// Test 4: AppendText
// ========================================================
TEST_CASE("AppendText appends correctly", "[webtextbox]")
{
    WebTextbox tb("Hello");

    tb.AppendText(", world!");
    CHECK(tb.GetText() == "Hello, world!", "append failed");
}

// ========================================================
// Test 5: Clear
// ========================================================
TEST_CASE("Clear empties text", "[webtextbox]")
{
    WebTextbox tb("something");

    tb.Clear();
    CHECK(tb.GetText().empty(), "text should be empty after Clear()");
}

// ========================================================
// Test 6: Show / Hide / IsVisible
// ========================================================
TEST_CASE("Show / Hide / IsVisible", "[webtextbox][visibility]")
{
    WebTextbox tb("x");

    CHECK(tb.IsVisible() == true, "default should be visible");
    tb.Hide();
    CHECK(tb.IsVisible() == false, "should be hidden after Hide()");
    tb.Show();
    CHECK(tb.IsVisible() == true, "should be visible after Show()");
}

// ========================================================
// Test 7: Multiple Show/Hide toggles
// ========================================================
TEST_CASE("Multiple Show/Hide toggles", "[webtextbox][visibility]")
{
    WebTextbox tb("x");

    for (int i = 0; i < 5; ++i)
    {
        tb.Hide();
        CHECK(tb.IsVisible() == false, "should be hidden");
        tb.Show();
        CHECK(tb.IsVisible() == true, "should be visible");
    }
}

// ========================================================
// Test 8: SyncFromModel (no crash, state preserved)
// ========================================================
TEST_CASE("SyncFromModel preserves state", "[webtextbox]")
{
    WebTextbox tb("hi");

    tb.SetFontFamily("Arial");
    tb.SetFontSize(18.0f);
    tb.SetBold(true);
    tb.SetItalic(true);
    tb.SetColor("#FF0000");
    tb.SetAlignment(WebTextbox::TextAlign::Center);
    tb.SetMaxWidth(300.0f);
    tb.SetWrap(true);
    tb.Hide();

    tb.SyncFromModel();

    CHECK(tb.GetText() == "hi", "text should remain");
    CHECK(tb.IsVisible() == false, "visibility should remain hidden");
}

// ========================================================
// Test 9: Bounding box returns 0 when not mounted
// ========================================================
TEST_CASE("Bounding box is zero when not mounted", "[webtextbox][bbox]")
{
    WebTextbox tb("measure me");

    auto r = tb.GetBoundingBoxPx();
    CHECK(r.w == 0.0, "width should be 0 when not mounted");
    CHECK(r.h == 0.0, "height should be 0 when not mounted");
}

// ========================================================
// Test 10: Id() uniqueness
// ========================================================
TEST_CASE("Id() is unique per instance", "[webtextbox][id]")
{
    WebTextbox a("a");
    WebTextbox b("b");
    WebTextbox c("c");

    CHECK(a.Id() != b.Id(), "a and b should have different ids");
    CHECK(b.Id() != c.Id(), "b and c should have different ids");
    CHECK(a.Id() != c.Id(), "a and c should have different ids");
}

// ========================================================
// Test 11: Id() prefix
// ========================================================
TEST_CASE("Id() starts with expected prefix", "[webtextbox][id]")
{
    WebTextbox tb("x");

    std::string id = tb.Id();
    CHECK(id.rfind("webtextbox-", 0) == 0,
          "id should start with 'webtextbox-'");
}

// ========================================================
// Test 12: Move constructor transfers state
// ========================================================
TEST_CASE("Move constructor transfers state", "[webtextbox][move]")
{
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

    CHECK(original.IsVisible() == false,
          "original should be hidden after move");
}

// ========================================================
// Test 13: Move assignment transfers state
// ========================================================
TEST_CASE("Move assignment transfers state", "[webtextbox][move]")
{
    WebTextbox src("source");
    src.SetBold(true);
    src.Hide();
    std::string src_id = src.Id();

    WebTextbox dest("dest");
    dest = std::move(src);

    CHECK(dest.GetText() == "source", "text should transfer");
    CHECK(dest.IsVisible() == false, "visibility should transfer");
    CHECK(dest.Id() == src_id, "id should transfer");
}

// ========================================================
// Test 14: Alignment accepts valid values
// ========================================================
TEST_CASE("SetAlignment accepts left/center/right", "[webtextbox]")
{
    WebTextbox tb("x");

    tb.SetAlignment(WebTextbox::TextAlign::Center);
    tb.SetAlignment(WebTextbox::TextAlign::Left);
    tb.SetAlignment(WebTextbox::TextAlign::Right);
}

// ========================================================
// Test 15: Background color set/clear
// ========================================================
TEST_CASE("SetBackgroundColor / ClearBackgroundColor do not crash",
          "[webtextbox][style]")
{
    WebTextbox tb("x");

    tb.SetBackgroundColor("#AABBCC");
    tb.ClearBackgroundColor();
    tb.SetBackgroundColor("transparent");
}

// ========================================================
// Test 16: Font fallback
// ========================================================
TEST_CASE("SetFallbackFontFamily does not crash", "[webtextbox][style]")
{
    WebTextbox tb("x");

    tb.SetFontFamily("DefinitelyNotARealFont");
    tb.SetFallbackFontFamily("serif");
}

// ========================================================
// Test 17: MaxWidth / Wrap interaction
// ========================================================
TEST_CASE("SetMaxWidth / SetWrap do not crash", "[webtextbox][style]")
{
    WebTextbox tb("long long long long long");

    tb.SetMaxWidth(200.0f);
    tb.SetWrap(true);
    tb.SetWrap(false);
}

// ========================================================
// Test 18: MountToLayout creates DOM element
// ========================================================
TEST_CASE("MountToLayout makes element appear in DOM",
          "[webtextbox][dom]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.SetSpacing(5);
    root.Apply();

    WebTextbox tb("hello");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    val el = ById(tb.Id());
    CHECK((!el.isNull() && !el.isUndefined()),
          "element should exist in DOM after mount");
}

// ========================================================
// Test 19: Unmount removes DOM element
// ========================================================
TEST_CASE("Unmount removes element from DOM", "[webtextbox][dom]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("bye");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    tb.Unmount();
    val el = ById(tb.Id());
    CHECK((el.isNull() || el.isUndefined()),
          "element should not exist after unmount");
}

// ========================================================
// Test 20: SetText updates DOM textContent when mounted
// ========================================================
TEST_CASE("SetText updates DOM textContent", "[webtextbox][dom]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("old");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    tb.SetText("new text");
    val el = ById(tb.Id());
    CHECK(el["textContent"].as<std::string>() == "new text",
          "textContent mismatch after SetText");
}

// ========================================================
// Test 21: AppendText updates DOM textContent when mounted
// ========================================================
TEST_CASE("AppendText updates DOM textContent", "[webtextbox][dom]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("A");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    tb.AppendText("B");
    val el = ById(tb.Id());
    CHECK(el["textContent"].as<std::string>() == "AB",
          "textContent mismatch after AppendText");
}

// ========================================================
// Test 22: Show/Hide affects computed display when mounted
// ========================================================
TEST_CASE("Show/Hide affects computed display", "[webtextbox][style]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("x");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    tb.Hide();
    val el = ById(tb.Id());
    std::string disp1 = GetComputedStyleStr(el, "display");
    CHECK(disp1 == "none", "display should be none after Hide()");

    tb.Show();
    std::string disp2 = GetComputedStyleStr(el, "display");
    CHECK(disp2 != "none", "display should not be none after Show()");
}

// ========================================================
// Test 23: SetFontSize affects computed font-size
// ========================================================
TEST_CASE("SetFontSize changes computed font-size",
          "[webtextbox][style]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("size");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    tb.SetFontSize(24.0f);
    val el = ById(tb.Id());
    std::string fs = GetComputedStyleStr(el, "font-size");
    CHECK(fs.find("24") != std::string::npos,
          "font-size should contain 24px");
}

// ========================================================
// Test 24: Bold/Italic affect computed styles
// ========================================================
TEST_CASE("SetBold/SetItalic change computed font-weight/style",
          "[webtextbox][style]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("style");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    tb.SetBold(true);
    tb.SetItalic(true);

    val el = ById(tb.Id());
    std::string fw = GetComputedStyleStr(el, "font-weight");
    std::string fs = GetComputedStyleStr(el, "font-style");

    CHECK((fw == "bold" || fw == "700" || fw == "800" || fw == "900"),
          "font-weight should be bold-ish");
    CHECK(fs == "italic", "font-style should be italic");
}

// ========================================================
// Test 25: SetColor affects computed color
// ========================================================
TEST_CASE("SetColor changes computed color", "[webtextbox][style]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("color");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    tb.SetColor("rgb(255, 0, 0)");
    val el = ById(tb.Id());
    std::string c = GetComputedStyleStr(el, "color");

    CHECK(c.find("255") != std::string::npos,
          "color should contain 255");
}

// ========================================================
// Test 26: SetAlignment affects computed text-align
// ========================================================
TEST_CASE("SetAlignment changes computed text-align",
          "[webtextbox][style]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("align");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    tb.SetAlignment(WebTextbox::TextAlign::Center);
    val el = ById(tb.Id());
    std::string ta = GetComputedStyleStr(el, "text-align");
    CHECK(ta == "center", "text-align should be center");
}

// ========================================================
// Test 27: SetMaxWidth affects computed max-width
// ========================================================
TEST_CASE("SetMaxWidth changes computed max-width",
          "[webtextbox][style]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("max width");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    tb.SetMaxWidth(123.0f);
    val el = ById(tb.Id());
    std::string mw = GetComputedStyleStr(el, "max-width");

    CHECK(mw.find("123") != std::string::npos,
          "max-width should contain 123px");
}

// ========================================================
// Test 28: Wrap toggles white-space computed property
// ========================================================
TEST_CASE("SetWrap toggles computed white-space",
          "[webtextbox][style]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("wrap");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    tb.SetWrap(true);
    val el = ById(tb.Id());
    std::string ws1 = GetComputedStyleStr(el, "white-space");
    CHECK((ws1.find("pre-wrap") != std::string::npos || ws1 == "pre-wrap"),
          "white-space should be pre-wrap when wrap enabled");

    tb.SetWrap(false);
    std::string ws2 = GetComputedStyleStr(el, "white-space");
    CHECK(ws2.find("pre") != std::string::npos,
          "white-space should be pre when wrap disabled");
}

// ========================================================
// Test 29: Background color applies via computed background-color
// ========================================================
TEST_CASE("Background color applies via computed background-color",
          "[webtextbox][style]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("bg");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    tb.SetBackgroundColor("rgb(1, 2, 3)");
    val el = ById(tb.Id());
    std::string bg = GetComputedStyleStr(el, "background-color");
    CHECK(bg.find("1") != std::string::npos,
          "background-color should contain 1");
}

// ========================================================
// Test 30: Bounding box becomes non-zero when mounted
// ========================================================
TEST_CASE("Bounding box is non-zero after mount",
          "[webtextbox][bbox]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("Hello bounding box");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    auto r = tb.GetBoundingBoxPx();
    CHECK(r.w > 0.0, "width should be > 0 after mount");
    CHECK(r.h > 0.0, "height should be > 0 after mount");
}

// ========================================================
// Test 31: Multi-line text increases height vs single line
// ========================================================
TEST_CASE("Multi-line text increases height", "[webtextbox][bbox]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("Line1");
    tb.SetFontSize(20.0f);
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    double h1 = tb.GetHeightPx();
    tb.SetText("Line1\nLine2\nLine3");
    root.Apply();

    double h2 = tb.GetHeightPx();
    CHECK(h2 > h1, "height should increase with multiple lines");
}

// ========================================================
// Test 32: Layout alignment sets align-self on the child
// ========================================================
TEST_CASE("Alignment sets align-self style", "[webtextbox][layout]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("align-self");
    tb.MountToLayout(root, Alignment::End);
    root.Apply();

    val el = ById(tb.Id());
    std::string as = GetComputedStyleStr(el, "align-self");

    CHECK((as == "flex-end" || as == "end"),
          "align-self should be flex-end-ish");
}

// ========================================================
// Test 33: SyncFromModel repairs DOM if style is manually changed
// ========================================================
TEST_CASE("SyncFromModel repairs manual DOM style mutation",
          "[webtextbox][dom]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("repair");
    tb.SetFontSize(18.0f);
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    val el = ById(tb.Id());
    // Mutate DOM directly
    el["style"].set("fontSize", std::string("2px"));
    std::string fs_bad = GetComputedStyleStr(el, "font-size");
    CHECK(fs_bad.find("2") != std::string::npos,
          "font-size should be 2px after mutation");

    // Repair from model
    tb.SyncFromModel();
    std::string fs_fixed = GetComputedStyleStr(el, "font-size");
    CHECK(fs_fixed.find("18") != std::string::npos,
          "font-size should be restored to ~18px");
}

// ========================================================
// Test 34: Move + mount keeps element accessible by same id
// ========================================================
TEST_CASE("Move then mount keeps element accessible",
          "[webtextbox][move][dom]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox a("move-dom");
    std::string id = a.Id();

    WebTextbox b(std::move(a));
    b.MountToLayout(root, Alignment::Start);
    root.Apply();

    val el = ById(id);
    CHECK((!el.isNull() && !el.isUndefined()),
          "moved element should exist by same id after mount");
    CHECK(el["textContent"].as<std::string>() == "move-dom",
          "textContent should remain after move");
}

// ========================================================
// Test 35: Two textboxes mount and preserve ordering in layout
// ========================================================
TEST_CASE("Layout preserves order of mounted textboxes",
          "[webtextbox][layout]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox a("A");
    WebTextbox b("B");
    a.MountToLayout(root, Alignment::Start);
    b.MountToLayout(root, Alignment::Start);
    root.Apply();

    val rootEl = ById(root.Id());
    val c0 = rootEl["children"][0];
    val c1 = rootEl["children"][1];

    CHECK(c0["id"].as<std::string>() == a.Id(),
          "first child should be textbox A");
    CHECK(c1["id"].as<std::string>() == b.Id(),
          "second child should be textbox B");
}

// ========================================================
// Test 36: Textbox can live inside a real HTML button
// ========================================================
TEST_CASE("Textbox can be embedded inside an HTML button",
          "[webtextbox][dom]")
{
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
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    val tbEl = ById(tb.Id());
    CHECK(!tbEl.isNull() && !tbEl.isUndefined(),
          "textbox element should exist");

    // Move textbox inside the button
    btn.call<void>("appendChild", tbEl);

    // Verify containment
    bool contains = btn.call<val>("contains", tbEl).as<bool>();
    CHECK(contains == true, "button should contain textbox element");

    // Verify combined visible text
    std::string combined = btn["textContent"].as<std::string>();
    CHECK(combined.find("Label") != std::string::npos,
          "button textContent should include textbox text");

    // cleanup: remove button
    doc["body"].call<void>("removeChild", btn);
}

// ========================================================
// Test 37: Clearing background returns transparent-ish
// ========================================================
TEST_CASE("ClearBackgroundColor results in transparent background",
          "[webtextbox][style]")
{
    WebLayout root(UniqueRootId("layout"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("bg-clear");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    tb.SetBackgroundColor("rgb(9, 9, 9)");
    tb.ClearBackgroundColor();

    val el = ById(tb.Id());
    std::string bg = GetComputedStyleStr(el, "background-color");

    CHECK((bg.find("rgba") != std::string::npos || bg == "transparent"),
          "background-color should be transparent-ish after clear");
}
// ========================================================
// Remaining tests mentioned in peer reviews are below
// ========================================================


// ========================================================
// Test 38: WebTextbox handles empty strings safely
// ========================================================

TEST_CASE("WebTextbox handles empty strings safely", "[webtextbox][edge]") {
    WebLayout root(UniqueRootId("layout-empty"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    WebTextbox tb("initial");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    // Set empty text
    tb.SetText("");
    val el = ById(tb.Id());
    CHECK(el["textContent"].as<std::string>().empty(),
          "textContent should be empty after SetText(\"\")");

    // Append empty text
    tb.AppendText("");
    CHECK(el["textContent"].as<std::string>().empty(),
          "AppendText(\"\") should not change empty text");
}

// ========================================================
// Test 39: WebTextbox handles very long text without crashing
// ========================================================

TEST_CASE("WebTextbox handles very long text", "[webtextbox][edge]") {
    WebLayout root(UniqueRootId("layout-long"));
    root.SetLayoutType(LayoutType::Vertical);
    root.Apply();

    // Create a very long string
    std::string longText(10000, 'X');

    WebTextbox tb("short");
    tb.MountToLayout(root, Alignment::Start);
    root.Apply();

    double h1 = tb.GetHeightPx();

    tb.SetText(longText);
    root.Apply();

    val el = ById(tb.Id());
    CHECK(el["textContent"].as<std::string>().size() == longText.size(),
          "DOM textContent should match long text size");

    double h2 = tb.GetHeightPx();
    CHECK(h2 >= h1, "height should not shrink after very long text");
}

#endif // __EMSCRIPTEN__