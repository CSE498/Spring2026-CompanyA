// File: tests/Interfaces/WebUI/WebCanvasTest.cpp
// Notes:
// - Do NOT define CATCH_CONFIG_MAIN here if another test file already defines it.

#include <type_traits>
#include <memory>
#include <vector>

//// For: tests/Interfaces/WebUI/WebCanvasTest.cpp
// #include "../../../third-party/Catch/single_include/catch2/catch.hpp"
// #include "../../../source/Interfaces/WebUI/WebCanvas.hpp"
// #include "../../../source/Interfaces/WebUI/internal/ICanvasElement.hpp"
//// For: group_specific_content/Group-18/WebUI/WebCanvas/WebCanvasTest.cpp
#define CATCH_CONFIG_MAIN
#include "../../../../third-party/Catch/single_include/catch2/catch.hpp"
#include "./WebCanvas.hpp"
#include "../internal/ICanvasElement.hpp"

// A minimal WebLayout stub for unit tests.
// This avoids pulling in the real WebLayout implementation (which may depend on emscripten).
class WebLayout {};

// ---------------------------
// Test helpers
// ---------------------------

struct SpyElement : public ICanvasElement {
    SpyElement(int id, std::vector<int>& order)
        : id_(id), order_(order) {}

    void draw(WebCanvas& /*canvas*/) override {
        ++drawCount;
        order_.push_back(id_);
    }

    // Directly mutate protected metadata for testing.
    void setVisible(bool v) { m_metadata.visible = v; }
    void setZIndex(int z) { m_metadata.zIndex = z; }

    int drawCount = 0;

private:
    int id_;
    std::vector<int>& order_;
};

struct LifetimeElement : public ICanvasElement {
    static int alive;

    LifetimeElement() { ++alive; }
    ~LifetimeElement() override { --alive; }

    void draw(WebCanvas& /*canvas*/) override {}
};

int LifetimeElement::alive = 0;

// ---------------------------
// Tests
// ---------------------------

TEST_CASE("WebCanvas is move-only (RAII-friendly)", "[web][canvas][raii]") {
    STATIC_REQUIRE(!std::is_copy_constructible_v<WebCanvas>);
    STATIC_REQUIRE(!std::is_copy_assignable_v<WebCanvas>);
    STATIC_REQUIRE(std::is_move_constructible_v<WebCanvas>);
    STATIC_REQUIRE(std::is_move_assignable_v<WebCanvas>);
}

TEST_CASE("WebCanvas implements IDomElement contract (compile-time)", "[web][canvas][dom]") {
    STATIC_REQUIRE(std::is_base_of_v<IDomElement, WebCanvas>);
}

TEST_CASE("WebCanvas DOM lifecycle calls are safe and id is stable", "[web][canvas][dom]") {
    WebCanvas canvas("canvas-test-1");
    WebLayout layout;

    REQUIRE(canvas.Id() == std::string("canvas-test-1"));

    REQUIRE_NOTHROW(canvas.mountToLayout(layout, Alignment::Center));
    REQUIRE_NOTHROW(canvas.syncFromModel());

    // Unmount should be safe to call multiple times (idempotent behavior is recommended).
    REQUIRE_NOTHROW(canvas.unmount());
    REQUIRE_NOTHROW(canvas.unmount());

    REQUIRE(canvas.Id() == std::string("canvas-test-1"));
}

TEST_CASE("WebCanvas owns added elements and releases them on clear/destroy", "[web][canvas][raii]") {
    REQUIRE(LifetimeElement::alive == 0);

    {
        WebCanvas canvas;
        canvas.addElement(std::make_unique<LifetimeElement>());
        canvas.addElement(std::make_unique<LifetimeElement>());

        REQUIRE(LifetimeElement::alive == 2);

        canvas.clearElements();
        REQUIRE(LifetimeElement::alive == 0);

        canvas.addElement(std::make_unique<LifetimeElement>());
        REQUIRE(LifetimeElement::alive == 1);
    }

    // After canvas destruction, all elements should be destroyed.
    REQUIRE(LifetimeElement::alive == 0);
}

TEST_CASE("WebCanvas renderFrame draws elements (baseline behavior)", "[web][canvas]") {
    WebCanvas canvas;

    std::vector<int> order;
    auto e1 = std::make_unique<SpyElement>(1, order);
    auto e2 = std::make_unique<SpyElement>(2, order);

    SpyElement* raw1 = e1.get();
    SpyElement* raw2 = e2.get();

    canvas.addElement(std::move(e1));
    canvas.addElement(std::move(e2));

    REQUIRE_NOTHROW(canvas.renderFrame());

    CHECK(raw1->drawCount == 1);
    CHECK(raw2->drawCount == 1);
    CHECK(order == std::vector<int>({1, 2}));
}

TEST_CASE("WebCanvas ignores null elements safely", "[web][canvas]") {
    WebCanvas canvas;
    std::unique_ptr<ICanvasElement> nullElem;
    REQUIRE_NOTHROW(canvas.addElement(std::move(nullElem)));
    REQUIRE_NOTHROW(canvas.renderFrame());
}

TEST_CASE("WebCanvas renderFrame respects visibility (TDD target)", "[web][canvas][visibility]") {
    WebCanvas canvas;

    std::vector<int> order;
    auto e1 = std::make_unique<SpyElement>(1, order);
    auto e2 = std::make_unique<SpyElement>(2, order);

    SpyElement* raw1 = e1.get();
    SpyElement* raw2 = e2.get();

    e2->setVisible(false);

    canvas.addElement(std::move(e1));
    canvas.addElement(std::move(e2));

    canvas.renderFrame();

    // Desired behavior: invisible elements should not draw.
    CHECK(raw1->drawCount == 1);
    CHECK(raw2->drawCount == 0);
    CHECK(order == std::vector<int>({1}));
}

TEST_CASE("WebCanvas renderFrame sorts by zIndex (stable) (TDD target)", "[web][canvas][zindex]") {
    WebCanvas canvas;

    std::vector<int> order;
    auto e1 = std::make_unique<SpyElement>(1, order);
    auto e2 = std::make_unique<SpyElement>(2, order);
    auto e3 = std::make_unique<SpyElement>(3, order);

    // Assign zIndex values (lower draws first).
    e1->setZIndex(10);
    e2->setZIndex(0);
    e3->setZIndex(10); // same as e1, should keep insertion order relative to e1

    canvas.addElement(std::move(e1)); // id=1, z=10
    canvas.addElement(std::move(e2)); // id=2, z=0
    canvas.addElement(std::move(e3)); // id=3, z=10

    canvas.renderFrame();

    // Desired order: z=0 first, then z=10 elements in insertion order (1 then 3).
    CHECK(order == std::vector<int>({2, 1, 3}));
}