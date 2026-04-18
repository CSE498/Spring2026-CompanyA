/**
 * @file WebCanvasTest.cpp
 * @brief Unit tests for the WebCanvas class using the Catch2 testing framework.
 *
 * These tests verify behavior including element ownership and lifetime,
 * rendering order, visibility filtering, z-index sorting, synchronization with
 * model state, immediate-mode drawing primitives, ID stability, and move
 * semantics when compiled to WebAssembly using Emscripten.
 *
 * Additional Note:
 *      Portions of formatting, documentation, and cleanup were assisted by
 *      AI tooling to improve consistency and readability.
 *
 * All project classes and tests correspond to the cse498 WebUI subsystem.
 *
 * Do NOT define CATCH_CONFIG_MAIN here if another test file already defines it.
 */

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../../../source/Interfaces/WebUI/WebCanvas/WebCanvas.hpp"
#include "../../../source/Interfaces/WebUI/internal/ICanvasElement.hpp"
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

using cse498::Alignment;
using cse498::ICanvasElement;
using cse498::WebCanvas;
// using namespace cse498;

// A minimal WebLayout stub for unit tests.
// This avoids pulling in the real WebLayout implementation (which may depend on emscripten).
class cse498::WebLayout {};

// ---------------------------
// Test helpers
// ---------------------------

struct SpyElement : public ICanvasElement {
    SpyElement(int id, std::vector<int>& order) : mId(id), order_(order) {}

    void Draw(WebCanvas& /*canvas*/) override {
        ++drawCount;
        order_.push_back(mId);
    }

    int drawCount = 0;

private:
    int mId;
    std::vector<int>& order_;
};

struct LifetimeElement : public ICanvasElement {
    inline static int alive = 0;
    inline static int destroyed = 0;

    LifetimeElement() { ++alive; }
    ~LifetimeElement() override {
        --alive;
        ++destroyed;
    }

    void Draw(WebCanvas& /*canvas*/) override {}

    static void reset() {
        alive = 0;
        destroyed = 0;
    }
};

struct OrderElement : public ICanvasElement {
    inline static std::vector<std::string> order;

    explicit OrderElement(std::string tag) : tag_(std::move(tag)) {}

    void Draw(WebCanvas& /*canvas*/) override { order.push_back(tag_); }

    static void reset() { order.clear(); }

private:
    std::string tag_;
};

// ---------------------------
// Tests
// ---------------------------


// ========================================================
// Test 1: WebCanvas is move-only (RAII-friendly)
// ========================================================
TEST_CASE("WebCanvas is move-only (RAII-friendly)", "[web][canvas][raii]") {
    STATIC_REQUIRE(!std::is_copy_constructible_v<WebCanvas>);
    STATIC_REQUIRE(!std::is_copy_assignable_v<WebCanvas>);
    STATIC_REQUIRE(std::is_move_constructible_v<WebCanvas>);
    STATIC_REQUIRE(std::is_move_assignable_v<WebCanvas>);

    WebCanvas a("move-a");
    a.AddElement(std::make_unique<LifetimeElement>());

    // Move-construct should not throw.
    REQUIRE_NOTHROW(WebCanvas{std::move(a)});

    // Moved-from object should remain safe to use (even if it becomes "empty").
    // We don't assert specific values, only that APIs remain valid.
    REQUIRE_NOTHROW(a.ClearElements());
    REQUIRE_NOTHROW(a.RenderFrame());
    REQUIRE_NOTHROW(a.Unmount());
    REQUIRE_NOTHROW(a.SyncFromModel());
    REQUIRE_NOTHROW(a.Id());
}


// ========================================================
// Test 2: Id remains stable across mount/unmount
// ========================================================
TEST_CASE("WebCanvas Id() reflects constructor argument, and mount/unmount do not change Id()", "[web][canvas][dom]") {
    WebCanvas canvas("canvas-test-1");
    cse498::WebLayout layout;

    REQUIRE(canvas.Id() == std::string("canvas-test-1"));

    // Id() returns a stable reference to internal string.
    const std::string* p1 = &canvas.Id();
    const std::string* p2 = &canvas.Id();
    REQUIRE(p1 == p2);

    REQUIRE_NOTHROW(canvas.MountToLayout(layout, Alignment::Start));
    REQUIRE(canvas.Id() == std::string("canvas-test-1"));

    REQUIRE_NOTHROW(canvas.Unmount());
    REQUIRE(canvas.Id() == std::string("canvas-test-1"));
}


// ========================================================
// Test 3: AddElement(nullptr) is safe
// ========================================================
TEST_CASE("WebCanvas AddElement(nullptr) is safe and does not affect ownership counts", "[web][canvas][raii]") {
    LifetimeElement::reset();
    REQUIRE(LifetimeElement::alive == 0);

    WebCanvas canvas("null-add");

    std::unique_ptr<ICanvasElement> nullPtr;
    REQUIRE_NOTHROW(canvas.AddElement(std::move(nullPtr)));
    REQUIRE(LifetimeElement::alive == 0);
    REQUIRE(LifetimeElement::destroyed == 0);

    canvas.AddElement(std::make_unique<LifetimeElement>());
    REQUIRE(LifetimeElement::alive == 1);

    // Adding another null still does nothing.
    std::unique_ptr<ICanvasElement> nullPtr2;
    REQUIRE_NOTHROW(canvas.AddElement(std::move(nullPtr2)));
    REQUIRE(LifetimeElement::alive == 1);
}


// ========================================================
// Test 4: Canvas owns elements and releases on clear/destroy
// ========================================================
TEST_CASE("WebCanvas owns added elements and releases them on clear/destroy (stronger assertions)",
          "[web][canvas][raii]") {
    LifetimeElement::reset();
    REQUIRE(LifetimeElement::alive == 0);
    REQUIRE(LifetimeElement::destroyed == 0);

    {
        WebCanvas canvas;

        canvas.AddElement(std::make_unique<LifetimeElement>());
        canvas.AddElement(std::make_unique<LifetimeElement>());

        REQUIRE(LifetimeElement::alive == 2);
        REQUIRE(LifetimeElement::destroyed == 0);

        // ClearElements should destroy owned elements.
        REQUIRE_NOTHROW(canvas.ClearElements());
        REQUIRE(LifetimeElement::alive == 0);
        REQUIRE(LifetimeElement::destroyed == 2);

        // ClearElements should be idempotent.
        REQUIRE_NOTHROW(canvas.ClearElements());
        REQUIRE(LifetimeElement::alive == 0);
        REQUIRE(LifetimeElement::destroyed == 2);

        // After clear, adding new elements works normally.
        canvas.AddElement(std::make_unique<LifetimeElement>());
        REQUIRE(LifetimeElement::alive == 1);
    }

    // After canvas destruction, all elements should be destroyed.
    REQUIRE(LifetimeElement::alive == 0);
    REQUIRE(LifetimeElement::destroyed == 3);
}


// ========================================================
// Test 5: RenderFrame respects visibility and preserves elements
// ========================================================
TEST_CASE("WebCanvas RenderFrame draws visible elements, skips invisible, and does not clear elements",
          "[web][canvas]") {
    WebCanvas canvas;

    std::vector<int> order;
    auto e1 = std::make_unique<SpyElement>(1, order);
    auto e2 = std::make_unique<SpyElement>(2, order);

    SpyElement* raw1 = e1.get();
    SpyElement* raw2 = e2.get();

    e2->SetVisible(false);

    canvas.AddElement(std::move(e1));
    canvas.AddElement(std::move(e2));

    REQUIRE_NOTHROW(canvas.RenderFrame());
    CHECK(raw1->drawCount == 1);
    CHECK(raw2->drawCount == 0);
    CHECK(order == std::vector<int>({1}));

    // Rendering again should draw again (elements are still owned).
    order.clear();
    REQUIRE_NOTHROW(canvas.RenderFrame());
    CHECK(raw1->drawCount == 2);
    CHECK(raw2->drawCount == 0);
    CHECK(order == std::vector<int>({1}));

    // Flip visibility after insertion should take effect on next frame.
    raw2->SetVisible(true);
    order.clear();
    REQUIRE_NOTHROW(canvas.RenderFrame());
    CHECK(raw1->drawCount == 3);
    CHECK(raw2->drawCount == 1);
    CHECK(order == std::vector<int>({1, 2}));
}


// ========================================================
// Test 6: RenderFrame sorts by ZIndex with stable ordering
// ========================================================
TEST_CASE("WebCanvas RenderFrame sorts by ZIndex (stable), including equal ZIndex insertion order",
          "[web][canvas][zindex]") {
    WebCanvas canvas;

    std::vector<int> order;
    auto e1 = std::make_unique<SpyElement>(1, order);
    auto e2 = std::make_unique<SpyElement>(2, order);
    auto e3 = std::make_unique<SpyElement>(3, order);
    auto e4 = std::make_unique<SpyElement>(4, order);

    // ZIndex values (lower draws first).
    e1->SetZIndex(10);
    e2->SetZIndex(0);
    e3->SetZIndex(10); // same as e1 -> should keep insertion order relative to e1
    e4->SetZIndex(10); // same as e1 -> after e3

    canvas.AddElement(std::move(e1)); // id=1, z=10
    canvas.AddElement(std::move(e2)); // id=2, z=0
    canvas.AddElement(std::move(e3)); // id=3, z=10
    canvas.AddElement(std::move(e4)); // id=4, z=10

    REQUIRE_NOTHROW(canvas.RenderFrame());

    // Desired order: z=0 first, then z=10 elements in insertion order (1 then 3 then 4).
    CHECK(order == std::vector<int>({2, 1, 3, 4}));
}


// ========================================================
// Test 7: ZIndex changes affect subsequent render order
// ========================================================
TEST_CASE("WebCanvas ZIndex changes after insertion affect subsequent render order", "[web][canvas][zindex]") {
    WebCanvas canvas;

    OrderElement::reset();
    auto a = std::make_unique<OrderElement>("A");
    auto b = std::make_unique<OrderElement>("B");
    auto c = std::make_unique<OrderElement>("C");

    OrderElement* ra = a.get();
    OrderElement* rb = b.get();
    OrderElement* rc = c.get();

    // initial z: all 0 -> insertion order
    canvas.AddElement(std::move(a));
    canvas.AddElement(std::move(b));
    canvas.AddElement(std::move(c));

    canvas.RenderFrame();
    REQUIRE(OrderElement::order == std::vector<std::string>({"A", "B", "C"}));

    // Now change ZIndex after insertion:
    // Make C draw first (z=-10), B draw last (z=10)
    rc->SetZIndex(-10);
    rb->SetZIndex(10);
    ra->SetZIndex(0);

    OrderElement::reset();
    canvas.RenderFrame();
    REQUIRE(OrderElement::order == std::vector<std::string>({"C", "A", "B"}));
}


// ========================================================
// Test 8: RenderFrame on empty canvas is safe
// ========================================================
TEST_CASE("WebCanvas RenderFrame on empty canvas is safe", "[web][canvas]") {
    WebCanvas canvas("empty");
    REQUIRE_NOTHROW(canvas.RenderFrame());
    REQUIRE_NOTHROW(canvas.ClearElements());
}


// ========================================================
// Test 9: Immediate-mode primitives are safe to call
// ========================================================
TEST_CASE("WebCanvas immediate-mode primitives are safe to call (expanded)", "[web][canvas][primitives]") {
    WebCanvas canvas("canvas-test-primitives");

    // These are no-ops in native builds; in Emscripten builds they forward to Canvas2D.
    // Either way, they should be safe to invoke with a wide range of inputs.

    // Clear: valid + edge cases
    REQUIRE_NOTHROW(canvas.Clear("#112233"));
    REQUIRE_NOTHROW(canvas.Clear("#000"));
    REQUIRE_NOTHROW(canvas.Clear("rgba(255,0,0,0.5)"));
    REQUIRE_NOTHROW(canvas.Clear("red"));
    REQUIRE_NOTHROW(canvas.Clear("")); // empty color string
    REQUIRE_NOTHROW(canvas.Clear("not-a-color")); // invalid but should not crash

    // Lines
    REQUIRE_NOTHROW(canvas.DrawLine(0, 0, 10, 10, 2.0f, "#ff00ff"));
    REQUIRE_NOTHROW(canvas.DrawLine(0, 0, 10, 10, 0.0f, "#ff00ff")); // 0 width

    // Circles / points
    REQUIRE_NOTHROW(canvas.DrawCircle(50, 50, 25, "#00ff00", 3.0f, "#001100"));
    REQUIRE_NOTHROW(canvas.DrawCircle(50, 50, 25, "#00ff00", 0.0f, "#001100")); // no stroke
    REQUIRE_NOTHROW(canvas.DrawCircle(50, 50, 25, "#00ff00", 3.0f, "")); // no fill
    REQUIRE_NOTHROW(canvas.DrawPoint(100, 100, 3.0f, "#ffffff"));

    // Polygon: normal + boundary cases
    std::vector<WebCanvas::Vec2> tri{{10, 10}, {60, 10}, {35, 60}};
    REQUIRE_NOTHROW(canvas.DrawPolygon(tri, "#ffffff", 1.5f, "#222222"));

    std::vector<WebCanvas::Vec2> empty;
    REQUIRE_NOTHROW(canvas.DrawPolygon(empty, "#fff", 1.0f, "#000")); // 0 points -> should early-return

    std::vector<WebCanvas::Vec2> one{{0, 0}};
    REQUIRE_NOTHROW(canvas.DrawPolygon(one, "#fff", 1.0f, "#000")); // 1 point -> should early-return

    std::vector<WebCanvas::Vec2> two{{0, 0}, {10, 0}};
    REQUIRE_NOTHROW(canvas.DrawPolygon(two, "#fff", 1.0f, "#000")); // 2 points -> allowed, implementation-defined
}


// ========================================================
// Test 10: DrawRect is safe across varied inputs
// ========================================================
TEST_CASE("WebCanvas DrawRect is safe to call with various inputs", "[web][canvas][primitives]") {
    WebCanvas canvas("canvas-test-drawrect");

    // Normal rect
    REQUIRE_NOTHROW(canvas.DrawRect(0, 0, 100, 50, "#ff0000"));
    // Zero-size rect
    REQUIRE_NOTHROW(canvas.DrawRect(10, 10, 0, 0, "#00ff00"));
    // Negative coordinates (allowed, implementation-defined on canvas)
    REQUIRE_NOTHROW(canvas.DrawRect(-5, -5, 20, 20, "#0000ff"));
    // Empty color string
    REQUIRE_NOTHROW(canvas.DrawRect(0, 0, 10, 10, ""));
    // Invalid color string (should not crash)
    REQUIRE_NOTHROW(canvas.DrawRect(0, 0, 10, 10, "not-a-color"));
}


// ========================================================
// Test 11: DrawText is safe across varied inputs
// ========================================================
TEST_CASE("WebCanvas DrawText is safe to call with various inputs", "[web][canvas][primitives]") {
    WebCanvas canvas("canvas-test-drawtext");

    REQUIRE_NOTHROW(canvas.DrawText(10, 20, "Hello", "#000000", 16.0f, "test"));
    // Empty text
    REQUIRE_NOTHROW(canvas.DrawText(10, 20, "", "#000000", 16.0f, "test"));
    // Zero font size
    REQUIRE_NOTHROW(canvas.DrawText(10, 20, "test", "#ff0000", 0.0f, "test"));
    // Empty color
    REQUIRE_NOTHROW(canvas.DrawText(0, 0, "x", "", 12.0f, "test"));
    // Large font
    REQUIRE_NOTHROW(canvas.DrawText(0, 0, "big", "#ffffff", 100.0f, "test"));
}


// ========================================================
// Test 12: DrawImage is safe across varied inputs
// ========================================================
TEST_CASE("WebCanvas DrawImage is safe to call with various inputs", "[web][canvas][primitives]") {
    WebCanvas canvas("canvas-test-drawimage");

    // Normal call
    REQUIRE_NOTHROW(canvas.DrawImage("sprite.png", 0, 0, 64, 64));
    // Negative size sentinel (-1 = natural size)
    REQUIRE_NOTHROW(canvas.DrawImage("sprite.png", 10, 10, -1, -1));
    // Empty src
    REQUIRE_NOTHROW(canvas.DrawImage("", 0, 0, 32, 32));
    // Zero position
    REQUIRE_NOTHROW(canvas.DrawImage("img.png", 0, 0, 0, 0));
}

// ========================================================
// Test 13: Default constructor falls back to "web-canvas" id
// ========================================================
TEST_CASE("WebCanvas default constructor uses fallback id", "[web][canvas][id]") {
    WebCanvas canvas;
    CHECK(canvas.Id() == std::string("web-canvas"));
}

// ========================================================
// Test 14: Move assignment transfers ownership safely
// ========================================================
TEST_CASE("WebCanvas move assignment is safe and preserves ownership", "[web][canvas][raii]") {
    LifetimeElement::reset();

    WebCanvas a("canvas-a");
    a.AddElement(std::make_unique<LifetimeElement>());
    a.AddElement(std::make_unique<LifetimeElement>());

    REQUIRE(LifetimeElement::alive == 2);

    WebCanvas b("canvas-b");
    b.AddElement(std::make_unique<LifetimeElement>());
    REQUIRE(LifetimeElement::alive == 3);

    REQUIRE_NOTHROW(b = std::move(a));

    // After move assignment, APIs should remain safe on both objects.
    REQUIRE_NOTHROW(a.ClearElements());
    REQUIRE_NOTHROW(a.RenderFrame());
    REQUIRE_NOTHROW(b.RenderFrame());

    // At least no leaks should occur by scope exit.
}

// ========================================================
// Test 15: SyncFromModel renders visible elements
// ========================================================
TEST_CASE("WebCanvas SyncFromModel calls RenderFrame", "[web][canvas][sync]") {
    WebCanvas canvas("sync-canvas");

    std::vector<int> order;
    auto e1 = std::make_unique<SpyElement>(1, order);
    auto e2 = std::make_unique<SpyElement>(2, order);

    SpyElement* raw1 = e1.get();
    SpyElement* raw2 = e2.get();

    e2->SetVisible(false);

    canvas.AddElement(std::move(e1));
    canvas.AddElement(std::move(e2));

    REQUIRE_NOTHROW(canvas.SyncFromModel());

    CHECK(raw1->drawCount == 1);
    CHECK(raw2->drawCount == 0);
    CHECK(order == std::vector<int>({1}));
}

// ========================================================
// Test 16: ClearElements removes all drawables before next render
// ========================================================
TEST_CASE("WebCanvas ClearElements prevents future draws", "[web][canvas][raii]") {
    WebCanvas canvas("clear-render-canvas");

    std::vector<int> order;
    auto e1 = std::make_unique<SpyElement>(1, order);
    auto e2 = std::make_unique<SpyElement>(2, order);

    SpyElement* raw1 = e1.get();
    SpyElement* raw2 = e2.get();

    canvas.AddElement(std::move(e1));
    canvas.AddElement(std::move(e2));

    canvas.RenderFrame();
    CHECK(raw1->drawCount == 1);
    CHECK(raw2->drawCount == 1);

    canvas.ClearElements();
    order.clear();

    REQUIRE_NOTHROW(canvas.RenderFrame());
    CHECK(order.empty());
}

// ========================================================
// Test 17: RenderFrame respects z-index and visibility together
// ========================================================
TEST_CASE("WebCanvas RenderFrame respects z-index and visibility together", "[web][canvas][zindex]") {
    WebCanvas canvas("z-vis-canvas");

    std::vector<int> order;
    auto e1 = std::make_unique<SpyElement>(1, order);
    auto e2 = std::make_unique<SpyElement>(2, order);
    auto e3 = std::make_unique<SpyElement>(3, order);

    e1->SetZIndex(5);
    e2->SetZIndex(-10);
    e3->SetZIndex(0);
    e3->SetVisible(false);

    canvas.AddElement(std::move(e1));
    canvas.AddElement(std::move(e2));
    canvas.AddElement(std::move(e3));

    REQUIRE_NOTHROW(canvas.RenderFrame());

    CHECK(order == std::vector<int>({2, 1}));
}

// ========================================================
// Test 18: DrawPoint is safe across varied inputs
// ========================================================
TEST_CASE("WebCanvas DrawPoint is safe to call with various inputs", "[web][canvas][primitives]") {
    WebCanvas canvas("drawpoint-test");

    REQUIRE_NOTHROW(canvas.DrawPoint(0, 0, 1.0f, "#ffffff"));
    REQUIRE_NOTHROW(canvas.DrawPoint(10, 20, 0.0f, "#ff0000"));
    REQUIRE_NOTHROW(canvas.DrawPoint(-5, -5, 3.5f, ""));
    REQUIRE_NOTHROW(canvas.DrawPoint(1000, 2000, 50.0f, "not-a-color"));
}

// ========================================================
// Test 19: DrawPolygon with fewer than two points is safe
// ========================================================
TEST_CASE("WebCanvas DrawPolygon safely ignores too-few points", "[web][canvas][primitives]") {
    WebCanvas canvas("polygon-early-return");

    std::vector<WebCanvas::Vec2> empty;
    std::vector<WebCanvas::Vec2> one{{42, 24}};

    REQUIRE_NOTHROW(canvas.DrawPolygon(empty, "#fff", 1.0f, "#000"));
    REQUIRE_NOTHROW(canvas.DrawPolygon(one, "#fff", 1.0f, "#000"));
}

// ========================================================
// Test 20: RenderFrame preserves insertion order when z-index is equal
// ========================================================
TEST_CASE("WebCanvas RenderFrame preserves insertion order for equal z-index", "[web][canvas][zindex]") {
    WebCanvas canvas("equal-z-canvas");

    OrderElement::reset();
    auto a = std::make_unique<OrderElement>("A");
    auto b = std::make_unique<OrderElement>("B");
    auto c = std::make_unique<OrderElement>("C");

    a->SetZIndex(7);
    b->SetZIndex(7);
    c->SetZIndex(7);

    canvas.AddElement(std::move(a));
    canvas.AddElement(std::move(b));
    canvas.AddElement(std::move(c));

    canvas.RenderFrame();

    CHECK(OrderElement::order == std::vector<std::string>({"A", "B", "C"}));
}
