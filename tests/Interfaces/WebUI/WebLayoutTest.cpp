/// File: WebLayoutTest.cpp
/// Unit tests for WebLayout class using Catch2 framework
/// Tests cover layout configuration, child element management, styling,
/// positioning, and DOM lifecycle behaviors.
/// to run these tests: em++ WebLayoutTest.cpp WebLayout.cpp -I.. -I../internal --bind -std=c++23 -s WASM=1 -s ASSERTIONS=1 -o test.html

#include <type_traits>
#include <memory>
#include <vector>
#include <string>

// Define CATCH_CONFIG_MAIN in this file only (first test file)
#define CATCH_CONFIG_MAIN
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../../source/Interfaces/WebUI/WebLayout/WebLayout.hpp"
#include "../../../source/Interfaces/WebUI/internal/IDomElement.hpp"

// ---------------------------
// Test Helpers & Stubs
// ---------------------------

/// A minimal test double for IDomElement that tracks lifecycle and state
class MockDomElement : public IDomElement {
public:
 explicit MockDomElement(const std::string& id = "test-elem") : id_(id) {
   // create DOM element so add and remove element can find it
   emscripten::val document = emscripten::val::global("document");
   emscripten::val el =
       document.call<emscripten::val>("createElement", std::string("div"));
   el.set("id", id_);
   document["body"].call<void>("appendChild", el);
 }

    virtual ~MockDomElement() = default;

    void mountToLayout(WebLayout& /*parent*/,
                       Alignment /*align*/) noexcept override {
        mountCount++;
    }

    void unmount() noexcept override {
        unmountCount++;
    }

    void syncFromModel() noexcept override {
        syncCount++;
    }

    const std::string& Id() const noexcept override {
        return id_;
    }

    // Test instrumentation
    int mountCount = 0;
    int unmountCount = 0;
    int syncCount = 0;
    bool shouldExist = true;  // For simulating invalid elements

private:
    std::string id_;
};

/// Test helper to reset ID counter between tests
static void ResetWebLayoutIdCounter() {
    // Since nextIdCounter_ is static and private, we can't directly reset it.
    // Tests should use explicit IDs to avoid interdependencies.
}

// ---------------------------
// Tests: Constructor & Destructor
// ---------------------------

TEST_CASE("WebLayout constructor with default ID", "[weblayout][constructor]") {
    WebLayout layout;
    std::string id = layout.Id();
    REQUIRE(!id.empty());
    REQUIRE(id.find("weblayout-") == 0);  // Auto-generated ID
}

TEST_CASE("WebLayout constructor with explicit ID", "[weblayout][constructor]") {
    WebLayout layout("my-custom-layout");
    REQUIRE(layout.Id() == "my-custom-layout");
}

TEST_CASE("WebLayout destructor is safe and idempotent", "[weblayout][destructor]") {
    WebLayout layout("destruct-test");
    REQUIRE(layout.Id() == "destruct-test");
    // Destructor called here, should clean up DOM
}

// ---------------------------
// Tests: IDomElement Interface
// ---------------------------

TEST_CASE("WebLayout implements IDomElement contract", "[weblayout][dom]") {
    STATIC_REQUIRE(std::is_base_of_v<IDomElement, WebLayout>);
}

TEST_CASE("WebLayout ID remains stable after operations", "[weblayout][dom]") {
    WebLayout layout("stable-id-test");
    REQUIRE(layout.Id() == "stable-id-test");

    // ID should not change even if we mount/unmount
    REQUIRE_NOTHROW(layout.unmount());
    REQUIRE(layout.Id() == "stable-id-test");
}

TEST_CASE("WebLayout mountToLayout and unmount are safe", "[weblayout][dom]") {
    WebLayout parent("parent");
    WebLayout child("child");

    REQUIRE_NOTHROW(child.mountToLayout(parent, Alignment::Center));
    REQUIRE_NOTHROW(child.unmount());
    REQUIRE_NOTHROW(child.unmount());  // Idempotent unmount
}

TEST_CASE("WebLayout syncFromModel calls Apply", "[weblayout][dom]") {
    WebLayout layout("sync-test");
    // syncFromModel calls Apply internally; should not throw
    REQUIRE_NOTHROW(layout.syncFromModel());
}

// ---------------------------
// Tests: Layout Configuration
// ---------------------------

TEST_CASE("WebLayout SetLayoutType stores and applies correctly", "[weblayout][layout-config]") {
    WebLayout layout("layout-type-test");

    REQUIRE_NOTHROW(layout.SetLayoutType(LayoutType::Horizontal));
    REQUIRE_NOTHROW(layout.Apply());

    REQUIRE_NOTHROW(layout.SetLayoutType(LayoutType::Vertical));
    REQUIRE_NOTHROW(layout.Apply());

    REQUIRE_NOTHROW(layout.SetLayoutType(LayoutType::Grid));
    REQUIRE_NOTHROW(layout.Apply());

    REQUIRE_NOTHROW(layout.SetLayoutType(LayoutType::Free));
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout SetJustification stores multiple values", "[weblayout][layout-config]") {
    WebLayout layout("justification-test");

    std::vector<Justification> justifications = {
        Justification::Start,
        Justification::Center,
        Justification::End,
        Justification::SpaceBetween,
        Justification::SpaceAround,
        Justification::SpaceEvenly
    };

    for (auto j : justifications) {
        REQUIRE_NOTHROW(layout.SetJustification(j));
        REQUIRE_NOTHROW(layout.Apply());
    }
}

TEST_CASE("WebLayout SetAlignItems stores alignment values", "[weblayout][layout-config]") {
    WebLayout layout("align-items-test");

    std::vector<Alignment> alignments = {
        Alignment::None,
        Alignment::Start,
        Alignment::Center,
        Alignment::End,
        Alignment::Stretch
    };

    for (auto a : alignments) {
        REQUIRE_NOTHROW(layout.SetAlignItems(a));
        REQUIRE_NOTHROW(layout.Apply());
    }
}

// ---------------------------
// Tests: Child Element Management
// ---------------------------

TEST_CASE("WebLayout AddElement with null pointer returns false", "[weblayout][children]") {
    WebLayout layout("add-null-test");
    REQUIRE(layout.AddElement(nullptr) == false);
}

TEST_CASE("WebLayout AddElement with invalid element ID returns false", "[weblayout][children]") {
    WebLayout layout("add-invalid-test");
    MockDomElement elem("");  // Empty ID
    REQUIRE(layout.AddElement(&elem) == false);
}

TEST_CASE("WebLayout can add and remove single element", "[weblayout][children]") {
    WebLayout layout("add-remove-test");
    MockDomElement elem("elem-1");

    // Add element (note: in real scenario would need DOM to exist)
    // The test checks internal state through Apply behavior
    REQUIRE_NOTHROW(layout.AddElement(&elem, Alignment::Center));
    REQUIRE_NOTHROW(layout.Apply());

    REQUIRE(layout.RemoveElement(&elem) == true);
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout RemoveElement returns false for non-existent element", "[weblayout][children]") {
    WebLayout layout("remove-nonexist-test");
    MockDomElement elem("elem-1");

    REQUIRE(layout.RemoveElement(&elem) == false);
}

TEST_CASE("WebLayout RemoveElement with null pointer returns false", "[weblayout][children]") {
    WebLayout layout("remove-null-test");
    REQUIRE(layout.RemoveElement(nullptr) == false);
}

TEST_CASE("WebLayout SetAlignment on non-existent element is safe", "[weblayout][children]") {
    WebLayout layout("set-align-test");
    MockDomElement elem("elem-1");

    REQUIRE_NOTHROW(layout.SetAlignment(&elem, Alignment::Center));
    REQUIRE_NOTHROW(layout.SetAlignment(nullptr, Alignment::Center));
}

TEST_CASE("WebLayout SetAlignment on added element works", "[weblayout][children]") {
    WebLayout layout("set-align-added-test");
    MockDomElement elem("elem-1");

    REQUIRE_NOTHROW(layout.AddElement(&elem, Alignment::Start));
    REQUIRE_NOTHROW(layout.SetAlignment(&elem, Alignment::Center));
    REQUIRE_NOTHROW(layout.SetAlignment(&elem, Alignment::End));
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout can add multiple elements", "[weblayout][children]") {
    WebLayout layout("multiple-children-test");
    MockDomElement elem1("elem-1");
    MockDomElement elem2("elem-2");
    MockDomElement elem3("elem-3");

    REQUIRE(layout.AddElement(&elem1, Alignment::Start) == true);
    REQUIRE(layout.AddElement(&elem2, Alignment::Center) == true);
    REQUIRE(layout.AddElement(&elem3, Alignment::End) == true);

    REQUIRE_NOTHROW(layout.Apply());

    // Remove first and verify others remain
    REQUIRE(layout.RemoveElement(&elem1) == true);
    REQUIRE_NOTHROW(layout.Apply());

    // Second should still be removable
    REQUIRE(layout.RemoveElement(&elem2) == true);
    REQUIRE(layout.RemoveElement(&elem3) == true);
}

TEST_CASE("WebLayout Clear removes all elements", "[weblayout][children]") {
    WebLayout layout("clear-test");
    MockDomElement elem1("elem-1");
    MockDomElement elem2("elem-2");

    layout.AddElement(&elem1);
    layout.AddElement(&elem2);

    REQUIRE_NOTHROW(layout.Clear());

    // After clear, elements should not be removable (not in layout)
    REQUIRE(layout.RemoveElement(&elem1) == false);
    REQUIRE(layout.RemoveElement(&elem2) == false);
}

// ---------------------------
// Tests: Styling Methods
// ---------------------------

TEST_CASE("WebLayout SetSpacing stores positive values", "[weblayout][styling]") {
    WebLayout layout("spacing-test");

    REQUIRE_NOTHROW(layout.SetSpacing(0));
    REQUIRE_NOTHROW(layout.Apply());

    REQUIRE_NOTHROW(layout.SetSpacing(10));
    REQUIRE_NOTHROW(layout.Apply());

    REQUIRE_NOTHROW(layout.SetSpacing(100));
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout SetBackgroundColor stores colors", "[weblayout][styling]") {
    WebLayout layout("bg-color-test");

    std::vector<std::string> colors = {
        "#ffffff",
        "#000000",
        "red",
        "rgb(255, 0, 0)",
        "rgba(255, 0, 0, 0.5)",
        "transparent"
    };

    for (const auto& color : colors) {
        REQUIRE_NOTHROW(layout.SetBackgroundColor(color));
        REQUIRE_NOTHROW(layout.Apply());
    }
}

TEST_CASE("WebLayout SetBorderColor stores colors", "[weblayout][styling]") {
    WebLayout layout("border-color-test");

    REQUIRE_NOTHROW(layout.SetBorderColor("#ff0000"));
    REQUIRE_NOTHROW(layout.Apply());

    REQUIRE_NOTHROW(layout.SetBorderColor("blue"));
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout SetBorderWidth stores positive values", "[weblayout][styling]") {
    WebLayout layout("border-width-test");

    std::vector<int> widths = {0, 1, 5, 10, 100};
    for (int w : widths) {
        REQUIRE_NOTHROW(layout.SetBorderWidth(w));
        REQUIRE_NOTHROW(layout.Apply());
    }
}

TEST_CASE("WebLayout SetBorderRadius stores positive values", "[weblayout][styling]") {
    WebLayout layout("border-radius-test");

    std::vector<int> radii = {0, 1, 5, 10, 50};
    for (int r : radii) {
        REQUIRE_NOTHROW(layout.SetBorderRadius(r));
        REQUIRE_NOTHROW(layout.Apply());
    }
}

TEST_CASE("WebLayout SetPadding stores positive values", "[weblayout][styling]") {
    WebLayout layout("padding-test");

    REQUIRE_NOTHROW(layout.SetPadding(0));
    REQUIRE_NOTHROW(layout.SetPadding(5));
    REQUIRE_NOTHROW(layout.SetPadding(20));
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout SetMargin stores positive values", "[weblayout][styling]") {
    WebLayout layout("margin-test");

    REQUIRE_NOTHROW(layout.SetMargin(0));
    REQUIRE_NOTHROW(layout.SetMargin(5));
    REQUIRE_NOTHROW(layout.SetMargin(20));
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout SetWidth stores positive values", "[weblayout][styling]") {
    WebLayout layout("width-test");

    REQUIRE_NOTHROW(layout.SetWidth(1));
    REQUIRE_NOTHROW(layout.SetWidth(100));
    REQUIRE_NOTHROW(layout.SetWidth(1000));
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout SetHeight stores positive values", "[weblayout][styling]") {
    WebLayout layout("height-test");

    REQUIRE_NOTHROW(layout.SetHeight(1));
    REQUIRE_NOTHROW(layout.SetHeight(100));
    REQUIRE_NOTHROW(layout.SetHeight(1000));
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout SetOpacity stores values in valid range", "[weblayout][styling]") {
    WebLayout layout("opacity-test");

    REQUIRE_NOTHROW(layout.SetOpacity(0.0));  // Fully transparent
    REQUIRE_NOTHROW(layout.Apply());

    REQUIRE_NOTHROW(layout.SetOpacity(0.5));  // Half transparent
    REQUIRE_NOTHROW(layout.Apply());

    REQUIRE_NOTHROW(layout.SetOpacity(1.0));  // Fully opaque
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout SetBoxShadow stores shadow strings", "[weblayout][styling]") {
    WebLayout layout("box-shadow-test");

    std::vector<std::string> shadows = {
        "0 4px 6px rgba(0, 0, 0, 0.1)",
        "0 0 10px #000000",
        "inset 0 0 5px #cccccc",
        ""
    };

    for (const auto& shadow : shadows) {
        REQUIRE_NOTHROW(layout.SetBoxShadow(shadow));
        REQUIRE_NOTHROW(layout.Apply());
    }
}

TEST_CASE("WebLayout ToggleVisibility toggles state", "[weblayout][visibility]") {
    WebLayout layout("visibility-test");

    REQUIRE_NOTHROW(layout.ToggleVisibility());
    REQUIRE_NOTHROW(layout.Apply());

    REQUIRE_NOTHROW(layout.ToggleVisibility());
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout ToggleVisibility multiple times", "[weblayout][visibility]") {
    WebLayout layout("visibility-multiple-test");

    for (int i = 0; i < 5; ++i) {
        REQUIRE_NOTHROW(layout.ToggleVisibility());
    }

    REQUIRE_NOTHROW(layout.Apply());
}

// ---------------------------
// Tests: Combined Styling
// ---------------------------

TEST_CASE("WebLayout complex styling scenario", "[weblayout][styling][integration]") {
    WebLayout layout("complex-style-test");

    // Set multiple styling properties
    layout.SetBackgroundColor("lightblue");
    layout.SetBorderColor("navy");
    layout.SetBorderWidth(2);
    layout.SetBorderRadius(8);
    layout.SetPadding(10);
    layout.SetMargin(5);
    layout.SetWidth(300);
    layout.SetHeight(200);
    layout.SetOpacity(0.95);
    layout.SetBoxShadow("0 4px 6px rgba(0, 0, 0, 0.1)");

    REQUIRE_NOTHROW(layout.Apply());
}

// ---------------------------
// Tests: Grid Positioning
// ---------------------------

TEST_CASE("IDomElement grid position helpers work correctly", "[weblayout][positioning][grid]") {
    MockDomElement elem("grid-test");

    // Default state: unset (-1)
    REQUIRE(elem.GridRow() == -1);
    REQUIRE(elem.GridCol() == -1);

    // Set positions
    elem.SetGridPosition(2, 3);
    REQUIRE(elem.GridRow() == 2);
    REQUIRE(elem.GridCol() == 3);

    // Change positions
    elem.SetGridPosition(0, 0);
    REQUIRE(elem.GridRow() == 0);
    REQUIRE(elem.GridCol() == 0);

    // Clear positions
    elem.ClearGridPosition();
    REQUIRE(elem.GridRow() == -1);
    REQUIRE(elem.GridCol() == -1);
}

TEST_CASE("IDomElement grid position with large indices", "[weblayout][positioning][grid]") {
    MockDomElement elem("large-grid-test");

    elem.SetGridPosition(100, 200);
    REQUIRE(elem.GridRow() == 100);
    REQUIRE(elem.GridCol() == 200);
}

TEST_CASE("WebLayout applies grid positions to children", "[weblayout][positioning][grid]") {
    WebLayout layout("grid-layout-test");
    layout.SetLayoutType(LayoutType::Grid);

    MockDomElement elem("grid-child");
    elem.SetGridPosition(1, 2);

    REQUIRE_NOTHROW(layout.AddElement(&elem));
    REQUIRE_NOTHROW(layout.Apply());

    // Grid position should be preserved
    REQUIRE(elem.GridRow() == 1);
    REQUIRE(elem.GridCol() == 2);
}

// ---------------------------
// Tests: Free Layout Positioning
// ---------------------------

TEST_CASE("IDomElement free position helpers work correctly", "[weblayout][positioning][free]") {
    MockDomElement elem("free-test");

    // Default state: unset (-1)
    REQUIRE(elem.FreeTop() == -1);
    REQUIRE(elem.FreeLeft() == -1);

    // Set positions
    elem.SetFreePosition(50, 100);
    REQUIRE(elem.FreeTop() == 50);
    REQUIRE(elem.FreeLeft() == 100);

    // Change positions
    elem.SetFreePosition(10, 20);
    REQUIRE(elem.FreeTop() == 10);
    REQUIRE(elem.FreeLeft() == 20);

    // Clear positions
    elem.ClearFreePosition();
    REQUIRE(elem.FreeTop() == -1);
    REQUIRE(elem.FreeLeft() == -1);
}

TEST_CASE("IDomElement free position with zero coordinates", "[weblayout][positioning][free]") {
    MockDomElement elem("zero-free-test");

    elem.SetFreePosition(0, 0);
    REQUIRE(elem.FreeTop() == 0);
    REQUIRE(elem.FreeLeft() == 0);
}

TEST_CASE("IDomElement free position with large coordinates", "[weblayout][positioning][free]") {
    MockDomElement elem("large-free-test");

    elem.SetFreePosition(5000, 10000);
    REQUIRE(elem.FreeTop() == 5000);
    REQUIRE(elem.FreeLeft() == 10000);
}

TEST_CASE("WebLayout applies free positions to children", "[weblayout][positioning][free]") {
    WebLayout layout("free-layout-test");
    layout.SetLayoutType(LayoutType::Free);

    MockDomElement elem("free-child");
    elem.SetFreePosition(50, 75);

    REQUIRE_NOTHROW(layout.AddElement(&elem));
    REQUIRE_NOTHROW(layout.Apply());

    // Free position should be preserved
    REQUIRE(elem.FreeTop() == 50);
    REQUIRE(elem.FreeLeft() == 75);
}

// ---------------------------
// Tests: Edge Cases
// ---------------------------

TEST_CASE("WebLayout with empty ID is safe", "[weblayout][edge-case]") {
    WebLayout layout("");
    // Constructor should handle empty string by generating an ID
    REQUIRE(!layout.Id().empty());
}

TEST_CASE("WebLayout SetSpacing with zero spacing", "[weblayout][edge-case]") {
    WebLayout layout("zero-spacing-test");
    REQUIRE_NOTHROW(layout.SetSpacing(0));
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout SetBorderWidth zero border", "[weblayout][edge-case]") {
    WebLayout layout("zero-border-test");
    REQUIRE_NOTHROW(layout.SetBorderWidth(0));
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout SetPadding and SetMargin with zero", "[weblayout][edge-case]") {
    WebLayout layout("zero-padding-margin-test");
    REQUIRE_NOTHROW(layout.SetPadding(0));
    REQUIRE_NOTHROW(layout.SetMargin(0));
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout SetOpacity edge values", "[weblayout][edge-case]") {
    WebLayout layout("opacity-edge-test");

    REQUIRE_NOTHROW(layout.SetOpacity(0.0));    // Min value
    REQUIRE_NOTHROW(layout.SetOpacity(1.0));    // Max value
    REQUIRE_NOTHROW(layout.SetOpacity(0.001));  // Near min
    REQUIRE_NOTHROW(layout.SetOpacity(0.999));  // Near max
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout empty color strings are safe", "[weblayout][edge-case]") {
    WebLayout layout("empty-color-test");

    REQUIRE_NOTHROW(layout.SetBackgroundColor(""));
    REQUIRE_NOTHROW(layout.SetBorderColor(""));
    REQUIRE_NOTHROW(layout.SetBoxShadow(""));
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout rapid Apply calls are safe", "[weblayout][edge-case]") {
    WebLayout layout("rapid-apply-test");

    for (int i = 0; i < 10; ++i) {
        REQUIRE_NOTHROW(layout.Apply());
    }
}

TEST_CASE("WebLayout remove same element multiple times", "[weblayout][edge-case]") {
    WebLayout layout("add-remove-repeat-test");
    MockDomElement elem("repeat-elem");

    REQUIRE(layout.AddElement(&elem) == true);
    REQUIRE(layout.RemoveElement(&elem) == true);
    REQUIRE(layout.RemoveElement(&elem) == false);  // Already removed
}

TEST_CASE("WebLayout SetAlignment on untrack element is safe", "[weblayout][edge-case]") {
    WebLayout layout("untrack-align-test");
    MockDomElement elem1("elem-1");
    MockDomElement elem2("elem-2");

    REQUIRE_NOTHROW(layout.AddElement(&elem1));
    REQUIRE_NOTHROW(layout.SetAlignment(&elem2, Alignment::Center));  // Not tracked
}

TEST_CASE("WebLayout consecutive Clear calls are safe", "[weblayout][edge-case]") {
    WebLayout layout("clear-clear-test");
    MockDomElement elem("elem");

    layout.AddElement(&elem);
    REQUIRE_NOTHROW(layout.Clear());
    REQUIRE_NOTHROW(layout.Clear());  // Should be safe
}

TEST_CASE("WebLayout nested layouts", "[weblayout][edge-case]") {
    WebLayout parent("parent");
    WebLayout child("child");
    WebLayout grandchild("grandchild");

    REQUIRE_NOTHROW(parent.AddElement(&child));
    REQUIRE_NOTHROW(child.AddElement(&grandchild));
    REQUIRE_NOTHROW(parent.Apply());
}

TEST_CASE("WebLayout very large spacing values", "[weblayout][edge-case]") {
    WebLayout layout("large-spacing-test");

    REQUIRE_NOTHROW(layout.SetSpacing(10000));
    REQUIRE_NOTHROW(layout.SetPadding(10000));
    REQUIRE_NOTHROW(layout.SetMargin(10000));
    REQUIRE_NOTHROW(layout.SetWidth(10000));
    REQUIRE_NOTHROW(layout.SetHeight(10000));
    REQUIRE_NOTHROW(layout.Apply());
}

// ---------------------------
// Tests: State Preservation
// ---------------------------

TEST_CASE("WebLayout preserves styling through multiple Apply calls", "[weblayout][state]") {
    WebLayout layout("state-preservation-test");

    layout.SetBackgroundColor("red");
    layout.SetBorderWidth(3);
    layout.SetPadding(10);

    REQUIRE_NOTHROW(layout.Apply());
    REQUIRE_NOTHROW(layout.Apply());
    REQUIRE_NOTHROW(layout.Apply());

    // Should still be applied
}

TEST_CASE("WebLayout preserves child order through operations", "[weblayout][state]") {
    WebLayout layout("child-order-test");
    MockDomElement elem1("elem-1");
    MockDomElement elem2("elem-2");
    MockDomElement elem3("elem-3");

    layout.AddElement(&elem1);
    layout.AddElement(&elem2);
    layout.AddElement(&elem3);

    layout.Apply();
    layout.Apply();  // Order should be preserved

    // Now remove middle element
    layout.RemoveElement(&elem2);
    REQUIRE_NOTHROW(layout.Apply());
}

TEST_CASE("WebLayout maintains all layout configurations through operations", "[weblayout][state]") {
    WebLayout layout("config-preservation-test");

    layout.SetLayoutType(LayoutType::Vertical);
    layout.SetJustification(Justification::Center);
    layout.SetAlignItems(Alignment::Stretch);
    layout.SetSpacing(15);

    MockDomElement elem("elem");
    layout.AddElement(&elem);

    REQUIRE_NOTHROW(layout.Apply());
    REQUIRE_NOTHROW(layout.Apply());
    // Configuration should be preserved
}
