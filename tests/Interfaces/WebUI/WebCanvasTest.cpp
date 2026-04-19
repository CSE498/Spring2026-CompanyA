/**
 * @file WebCanvasTest.cpp
 * @brief Focused tests for WebCanvas.
 *
 * Rationale for this version:
 * - Keep the core tests centered on observable C++ behavior that belongs in a
 *   unit test: ownership, lifetime, visibility filtering, stable z-ordering,
 *   and SyncFromModel dispatch.
 * - Avoid using REQUIRE_NOTHROW as the primary proof of correctness.
 * - In browser/Emscripten builds, add only a thin DOM-backed smoke layer:
 *   ensure a real <canvas> exists for tests that construct WebCanvas with a
 *   custom id, and include one minimal real-canvas verification so the test
 *   suite still touches the DOM boundary without recreating the demo logic.
 */

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../../../source/Interfaces/WebUI/WebCanvas/WebCanvas.hpp"
#include "../../../source/Interfaces/WebUI/internal/ICanvasElement.hpp"
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

using cse498::ICanvasElement;
using cse498::WebCanvas;

#ifndef __EMSCRIPTEN__
class cse498::WebLayout {};
#endif

namespace {

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

    static void Reset() {
        alive = 0;
        destroyed = 0;
    }
};

struct OrderElement : public ICanvasElement {
    inline static std::vector<std::string> order;

    explicit OrderElement(std::string tag) : tag_(std::move(tag)) {}

    void Draw(WebCanvas& /*canvas*/) override { order.push_back(tag_); }

    static void Reset() { order.clear(); }

private:
    std::string tag_;
};

#ifdef __EMSCRIPTEN__

void EnsureDomCanvas(const std::string& id, int cssWidth = 320, int cssHeight = 180) {
    using emscripten::val;

    val document = val::global("document");
    val existing = document.call<val>("getElementById", val(id));
    if (!existing.isNull() && !existing.isUndefined()) {
        existing.call<void>("remove");
    }

    val canvas = document.call<val>("createElement", val("canvas"));
    canvas.set("id", val(id));
    canvas.call<void>("setAttribute", val("style"),
                      val("width:" + std::to_string(cssWidth) + "px;height:" + std::to_string(cssHeight) +
                          "px;display:block;"));
    document["body"].call<void>("appendChild", canvas);
}

void RemoveDomCanvas(const std::string& id) {
    using emscripten::val;

    val document = val::global("document");
    val existing = document.call<val>("getElementById", val(id));
    if (!existing.isNull() && !existing.isUndefined()) {
        existing.call<void>("remove");
    }
}

struct DomCanvasFixture {
    explicit DomCanvasFixture(std::string canvasId, int cssWidth = 320, int cssHeight = 180)
        : id(std::move(canvasId)) {
        EnsureDomCanvas(id, cssWidth, cssHeight);
    }

    ~DomCanvasFixture() { RemoveDomCanvas(id); }

    std::string id;
};

struct CanvasHarness {
    explicit CanvasHarness(std::string canvasId) : fixture(std::move(canvasId)), canvas(fixture.id) {}

    DomCanvasFixture fixture;
    WebCanvas canvas;
};

std::array<unsigned, 4> ReadPixelRGBA(const std::string& id, int x, int y) {
    using emscripten::val;

    val document = val::global("document");
    val canvas = document.call<val>("getElementById", val(id));
    val ctx = canvas.call<val>("getContext", val("2d"));
    val data = ctx.call<val>("getImageData", x, y, 1, 1)["data"];

    return {data[0].as<unsigned>(), data[1].as<unsigned>(), data[2].as<unsigned>(), data[3].as<unsigned>()};
}

#else

struct CanvasHarness {
    explicit CanvasHarness(const std::string& canvasId) : canvas(canvasId) {}

    WebCanvas canvas;
};

#endif

} // namespace

TEST_CASE("WebCanvas is move-only", "[web][canvas][traits]") {
    STATIC_REQUIRE(!std::is_copy_constructible_v<WebCanvas>);
    STATIC_REQUIRE(!std::is_copy_assignable_v<WebCanvas>);
    STATIC_REQUIRE(std::is_move_constructible_v<WebCanvas>);
    STATIC_REQUIRE(std::is_move_assignable_v<WebCanvas>);
}

TEST_CASE("WebCanvas default constructor uses fallback id", "[web][canvas][id]") {
#ifdef __EMSCRIPTEN__
    DomCanvasFixture fixture("web-canvas");
#endif
    WebCanvas canvas;
    CHECK(canvas.Id() == std::string("web-canvas"));
}

TEST_CASE("WebCanvas Id() reflects explicit constructor id", "[web][canvas][id]") {
    CanvasHarness harness("canvas-test-1");
    CHECK(harness.canvas.Id() == std::string("canvas-test-1"));

    const std::string* p1 = &harness.canvas.Id();
    const std::string* p2 = &harness.canvas.Id();
    CHECK(p1 == p2);
}

TEST_CASE("WebCanvas AddElement(nullptr) is ignored", "[web][canvas][raii]") {
    LifetimeElement::Reset();
    CanvasHarness harness("null-add");

    std::unique_ptr<ICanvasElement> nullPtr;
    harness.canvas.AddElement(std::move(nullPtr));

    CHECK(LifetimeElement::alive == 0);
    CHECK(LifetimeElement::destroyed == 0);

    harness.canvas.AddElement(std::make_unique<LifetimeElement>());
    CHECK(LifetimeElement::alive == 1);

    std::unique_ptr<ICanvasElement> nullPtr2;
    harness.canvas.AddElement(std::move(nullPtr2));

    CHECK(LifetimeElement::alive == 1);
    CHECK(LifetimeElement::destroyed == 0);
}

TEST_CASE("WebCanvas owns added elements and releases them on clear and destroy", "[web][canvas][raii]") {
    LifetimeElement::Reset();

    {
#ifdef __EMSCRIPTEN__
        DomCanvasFixture fixture("own-clear");
        WebCanvas canvas(fixture.id);
#else
        WebCanvas canvas("own-clear");
#endif
        canvas.AddElement(std::make_unique<LifetimeElement>());
        canvas.AddElement(std::make_unique<LifetimeElement>());

        REQUIRE(LifetimeElement::alive == 2);
        REQUIRE(LifetimeElement::destroyed == 0);

        canvas.ClearElements();
        CHECK(LifetimeElement::alive == 0);
        CHECK(LifetimeElement::destroyed == 2);

        canvas.AddElement(std::make_unique<LifetimeElement>());
        CHECK(LifetimeElement::alive == 1);
    }

    CHECK(LifetimeElement::alive == 0);
    CHECK(LifetimeElement::destroyed == 3);
}

TEST_CASE("WebCanvas move construction transfers ownership without leaks", "[web][canvas][raii][move]") {
    LifetimeElement::Reset();

    {
#ifdef __EMSCRIPTEN__
        DomCanvasFixture fixture("move-source");
        WebCanvas source(fixture.id);
#else
        WebCanvas source("move-source");
#endif
        source.AddElement(std::make_unique<LifetimeElement>());
        source.AddElement(std::make_unique<LifetimeElement>());
        REQUIRE(LifetimeElement::alive == 2);

        WebCanvas moved(std::move(source));
        CHECK(LifetimeElement::alive == 2);

        moved.ClearElements();
        CHECK(LifetimeElement::alive == 0);
        CHECK(LifetimeElement::destroyed == 2);
    }

    CHECK(LifetimeElement::alive == 0);
    CHECK(LifetimeElement::destroyed == 2);
}

TEST_CASE("WebCanvas move assignment replaces previous ownership correctly", "[web][canvas][raii][move]") {
    LifetimeElement::Reset();

    {
#ifdef __EMSCRIPTEN__
        DomCanvasFixture fixtureA("canvas-a");
        DomCanvasFixture fixtureB("canvas-b");
        WebCanvas a(fixtureA.id);
        WebCanvas b(fixtureB.id);
#else
        WebCanvas a("canvas-a");
        WebCanvas b("canvas-b");
#endif
        a.AddElement(std::make_unique<LifetimeElement>());
        a.AddElement(std::make_unique<LifetimeElement>());
        b.AddElement(std::make_unique<LifetimeElement>());

        REQUIRE(LifetimeElement::alive == 3);
        REQUIRE(LifetimeElement::destroyed == 0);

        b = std::move(a);

        CHECK(LifetimeElement::alive == 2);
        CHECK(LifetimeElement::destroyed == 1);
    }

    CHECK(LifetimeElement::alive == 0);
    CHECK(LifetimeElement::destroyed == 3);
}

TEST_CASE("WebCanvas RenderFrame draws visible elements, skips invisible, and preserves ownership",
          "[web][canvas][render]") {
    CanvasHarness harness("render-frame");

    std::vector<int> order;
    auto e1 = std::make_unique<SpyElement>(1, order);
    auto e2 = std::make_unique<SpyElement>(2, order);

    SpyElement* raw1 = e1.get();
    SpyElement* raw2 = e2.get();

    e2->SetVisible(false);

    harness.canvas.AddElement(std::move(e1));
    harness.canvas.AddElement(std::move(e2));

    harness.canvas.RenderFrame();
    CHECK(raw1->drawCount == 1);
    CHECK(raw2->drawCount == 0);
    CHECK(order == std::vector<int>{1});

    order.clear();
    harness.canvas.RenderFrame();
    CHECK(raw1->drawCount == 2);
    CHECK(raw2->drawCount == 0);
    CHECK(order == std::vector<int>{1});

    raw2->SetVisible(true);
    order.clear();
    harness.canvas.RenderFrame();
    CHECK(raw1->drawCount == 3);
    CHECK(raw2->drawCount == 1);
    CHECK(order == std::vector<int>{1, 2});
}

TEST_CASE("WebCanvas RenderFrame sorts by ZIndex and preserves insertion order for ties",
          "[web][canvas][zindex]") {
    CanvasHarness harness("zindex-sort");

    std::vector<int> order;
    auto e1 = std::make_unique<SpyElement>(1, order);
    auto e2 = std::make_unique<SpyElement>(2, order);
    auto e3 = std::make_unique<SpyElement>(3, order);
    auto e4 = std::make_unique<SpyElement>(4, order);

    e1->SetZIndex(10);
    e2->SetZIndex(0);
    e3->SetZIndex(10);
    e4->SetZIndex(10);

    harness.canvas.AddElement(std::move(e1));
    harness.canvas.AddElement(std::move(e2));
    harness.canvas.AddElement(std::move(e3));
    harness.canvas.AddElement(std::move(e4));

    harness.canvas.RenderFrame();

    CHECK(order == std::vector<int>{2, 1, 3, 4});
}

TEST_CASE("WebCanvas ZIndex changes after insertion affect subsequent render order", "[web][canvas][zindex]") {
    CanvasHarness harness("zindex-update");

    OrderElement::Reset();
    auto a = std::make_unique<OrderElement>("A");
    auto b = std::make_unique<OrderElement>("B");
    auto c = std::make_unique<OrderElement>("C");

    OrderElement* ra = a.get();
    OrderElement* rb = b.get();
    OrderElement* rc = c.get();

    harness.canvas.AddElement(std::move(a));
    harness.canvas.AddElement(std::move(b));
    harness.canvas.AddElement(std::move(c));

    harness.canvas.RenderFrame();
    REQUIRE(OrderElement::order == std::vector<std::string>{"A", "B", "C"});

    rc->SetZIndex(-10);
    rb->SetZIndex(10);
    ra->SetZIndex(0);

    OrderElement::Reset();
    harness.canvas.RenderFrame();

    CHECK(OrderElement::order == std::vector<std::string>{"C", "A", "B"});
}

TEST_CASE("WebCanvas ClearElements prevents future draws", "[web][canvas][raii]") {
    CanvasHarness harness("clear-render-canvas");

    std::vector<int> order;
    auto e1 = std::make_unique<SpyElement>(1, order);
    auto e2 = std::make_unique<SpyElement>(2, order);

    SpyElement* raw1 = e1.get();
    SpyElement* raw2 = e2.get();

    harness.canvas.AddElement(std::move(e1));
    harness.canvas.AddElement(std::move(e2));

    harness.canvas.RenderFrame();
    REQUIRE(raw1->drawCount == 1);
    REQUIRE(raw2->drawCount == 1);
    CHECK(order == std::vector<int>{1, 2});

    harness.canvas.ClearElements();
    order.clear();

    harness.canvas.RenderFrame();
    CHECK(order.empty());
}

TEST_CASE("WebCanvas SyncFromModel delegates to RenderFrame semantics", "[web][canvas][sync]") {
    CanvasHarness harness("sync-canvas");

    std::vector<int> order;
    auto e1 = std::make_unique<SpyElement>(1, order);
    auto e2 = std::make_unique<SpyElement>(2, order);

    SpyElement* raw1 = e1.get();
    SpyElement* raw2 = e2.get();

    e2->SetVisible(false);
    harness.canvas.AddElement(std::move(e1));
    harness.canvas.AddElement(std::move(e2));

    harness.canvas.SyncFromModel();

    CHECK(raw1->drawCount == 1);
    CHECK(raw2->drawCount == 0);
    CHECK(order == std::vector<int>{1});
}

#ifdef __EMSCRIPTEN__

TEST_CASE("WebCanvas can bind to an existing DOM canvas in browser builds", "[web][canvas][dom][smoke]") {
    DomCanvasFixture fixture("dom-bind-canvas", 240, 120);

    WebCanvas canvas(fixture.id);

    emscripten::val document = emscripten::val::global("document");
    emscripten::val canvasEl = document.call<emscripten::val>("getElementById", emscripten::val(fixture.id));

    REQUIRE(!canvasEl.isNull());
    CHECK(canvas.Id() == fixture.id);
    CHECK(canvasEl["width"].as<int>() > 0);
    CHECK(canvasEl["height"].as<int>() > 0);
}

TEST_CASE("WebCanvas DrawRect affects pixels on a real DOM canvas", "[web][canvas][dom][smoke]") {
    DomCanvasFixture fixture("dom-rect-smoke", 64, 64);
    WebCanvas canvas(fixture.id);

    canvas.DrawRect(0.0f, 0.0f, 16.0f, 16.0f, "#ff0000");

    const auto pixel = ReadPixelRGBA(fixture.id, 2, 2);
    CHECK(pixel[0] == 255);
    CHECK(pixel[1] == 0);
    CHECK(pixel[2] == 0);
    CHECK(pixel[3] > 0);
}

#endif // __EMSCRIPTEN__
