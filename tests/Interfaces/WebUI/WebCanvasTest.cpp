/**
 * @file WebCanvasTest.cpp
 * @brief Unit tests for WebCanvas, the DOM-backed 2D drawing surface.
 *
 * Do NOT define CATCH_CONFIG_MAIN here if another test file already defines it.
 *
 */

#include <type_traits>
#include <memory>
#include <vector>
#include <string>
#include <utility>

#include "../../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../../source/Interfaces/WebUI/WebCanvas/WebCanvas.hpp"
#include "../../../source/Interfaces/WebUI/internal/ICanvasElement.hpp"

using cse498::WebCanvas;
using cse498::ICanvasElement;
using cse498::Alignment;
// using namespace cse498;

// A minimal WebLayout stub for unit tests.
// This avoids pulling in the real WebLayout implementation (which may depend on emscripten).
class cse498::WebLayout {};

// ---------------------------
// Test helpers
// ---------------------------

struct SpyElement : public ICanvasElement {
    SpyElement(int id, std::vector<int>& order)
        : mId(id), order_(order) {}

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
    ~LifetimeElement() override { --alive; ++destroyed; }

    void Draw(WebCanvas& /*canvas*/) override {}

    static void reset() { alive = 0; destroyed = 0; }
};

struct OrderElement : public ICanvasElement {
    inline static std::vector<std::string> order;

    explicit OrderElement(std::string tag) : tag_(std::move(tag)) {}

    void Draw(WebCanvas& /*canvas*/) override {
        order.push_back(tag_);
    }

    static void reset() { order.clear(); }

private:
    std::string tag_;
};

// ---------------------------
// Tests
// ---------------------------

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

TEST_CASE("WebCanvas owns added elements and releases them on clear/destroy (stronger assertions)", "[web][canvas][raii]") {
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

TEST_CASE("WebCanvas RenderFrame draws visible elements, skips invisible, and does not clear elements", "[web][canvas]") {
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

TEST_CASE("WebCanvas RenderFrame sorts by ZIndex (stable), including equal ZIndex insertion order", "[web][canvas][zindex]") {
    WebCanvas canvas;

    std::vector<int> order;
    auto e1 = std::make_unique<SpyElement>(1, order);
    auto e2 = std::make_unique<SpyElement>(2, order);
    auto e3 = std::make_unique<SpyElement>(3, order);
    auto e4 = std::make_unique<SpyElement>(4, order);

    // ZIndex values (lower draws first).
    e1->SetZIndex(10);
    e2->SetZIndex(0);
    e3->SetZIndex(10);  // same as e1 -> should keep insertion order relative to e1
    e4->SetZIndex(10);  // same as e1 -> after e3

    canvas.AddElement(std::move(e1)); // id=1, z=10
    canvas.AddElement(std::move(e2)); // id=2, z=0
    canvas.AddElement(std::move(e3)); // id=3, z=10
    canvas.AddElement(std::move(e4)); // id=4, z=10

    REQUIRE_NOTHROW(canvas.RenderFrame());

    // Desired order: z=0 first, then z=10 elements in insertion order (1 then 3 then 4).
    CHECK(order == std::vector<int>({2, 1, 3, 4}));
}

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

TEST_CASE("WebCanvas RenderFrame on empty canvas is safe", "[web][canvas]") {
    WebCanvas canvas("empty");
    REQUIRE_NOTHROW(canvas.RenderFrame());
    REQUIRE_NOTHROW(canvas.ClearElements());
}

TEST_CASE("WebCanvas immediate-mode primitives are safe to call (expanded)", "[web][canvas][primitives]") {
    WebCanvas canvas("canvas-test-primitives");

    // These are no-ops in native builds; in Emscripten builds they forward to Canvas2D.
    // Either way, they should be safe to invoke with a wide range of inputs.

    // Clear: valid + edge cases
    REQUIRE_NOTHROW(canvas.Clear("#112233"));
    REQUIRE_NOTHROW(canvas.Clear("#000"));
    REQUIRE_NOTHROW(canvas.Clear("rgba(255,0,0,0.5)"));
    REQUIRE_NOTHROW(canvas.Clear("red"));
    REQUIRE_NOTHROW(canvas.Clear(""));              // empty color string
    REQUIRE_NOTHROW(canvas.Clear("not-a-color"));   // invalid but should not crash

    // Lines
    REQUIRE_NOTHROW(canvas.DrawLine(0, 0, 10, 10, 2.0f, "#ff00ff"));
    REQUIRE_NOTHROW(canvas.DrawLine(0, 0, 10, 10, 0.0f, "#ff00ff")); // 0 width

    // Circles / points
    REQUIRE_NOTHROW(canvas.DrawCircle(50, 50, 25, "#00ff00", 3.0f, "#001100"));
    REQUIRE_NOTHROW(canvas.DrawCircle(50, 50, 25, "#00ff00", 0.0f, "#001100")); // no stroke
    REQUIRE_NOTHROW(canvas.DrawCircle(50, 50, 25, "#00ff00", 3.0f, ""));        // no fill
    REQUIRE_NOTHROW(canvas.DrawPoint(100, 100, 3.0f, "#ffffff"));

    // Polygon: normal + boundary cases
    std::vector<WebCanvas::Vec2> tri{{10,10},{60,10},{35,60}};
    REQUIRE_NOTHROW(canvas.DrawPolygon(tri, "#ffffff", 1.5f, "#222222"));

    std::vector<WebCanvas::Vec2> empty;
    REQUIRE_NOTHROW(canvas.DrawPolygon(empty, "#fff", 1.0f, "#000")); // 0 points -> should early-return

    std::vector<WebCanvas::Vec2> one{{0,0}};
    REQUIRE_NOTHROW(canvas.DrawPolygon(one, "#fff", 1.0f, "#000"));   // 1 point -> should early-return

    std::vector<WebCanvas::Vec2> two{{0,0},{10,0}};
    REQUIRE_NOTHROW(canvas.DrawPolygon(two, "#fff", 1.0f, "#000"));   // 2 points -> allowed, implementation-defined
}