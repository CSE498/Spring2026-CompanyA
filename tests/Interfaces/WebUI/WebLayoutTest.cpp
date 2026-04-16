/**
 * @file WebLayoutTest.cpp
 * @brief Unit tests for the WebLayout class using the Catch2 testing framework.
 *
 * These tests verify behavior including layout construction, element mounting
 * and unmounting, alignment and layout type handling, child management,
 * DOM integration, ordering of elements, and synchronization with layout state
 * when compiled to WebAssembly using Emscripten.
 *
 * Additional Note:
 *      Portions of formatting, documentation, and cleanup were assisted by
 *      AI tooling to improve consistency and readability.
 *
 * All project classes and tests correspond to the cse498 WebUI subsystem.
 *
 */

#ifdef __EMSCRIPTEN__

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

// Define CATCH_CONFIG_MAIN in this file only (first test file)
// #define CATCH_CONFIG_MAIN
#include "../../../source/Interfaces/WebUI/WebLayout/WebLayout.hpp"
#include "../../../source/Interfaces/WebUI/WebUtils.hpp"
#include "../../../source/Interfaces/WebUI/internal/IDomElement.hpp"
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"
#include "./SharedWebContext.hpp"

using namespace cse498;

// ---------------------------
// Test Helpers & Stubs
// ---------------------------

/// A minimal test double for IDomElement that tracks lifecycle and state
class MockDomElement : public IDomElement {
public:
    explicit MockDomElement(const std::string& id = "mock-element") {
        mId = id;
        // create DOM element so add and remove element can find it
        mElement = GetDocument().call<emscripten::val>("createElement", std::string("div"));
        mElement.set("id", mId);
    }

    virtual ~MockDomElement() {
        if (mId.empty())
            return;

        Unmount();
    };

    void MountToLayout(WebLayout& parent, Alignment align = Alignment::None) noexcept override {
        if (parent.AddElement(this, align))
            mountCount++;
    }

    void Unmount() noexcept override {
        IDomElement::Unmount();
        unmountCount++;
    }

    void SyncFromModel() noexcept override { syncCount++; }

    // Test instrumentation
    int mountCount = 0;
    int unmountCount = 0;
    int syncCount = 0;
};

/// Test helper to reset ID counter between tests
// static void ResetWebLayoutIdCounter() {
//     // Since mNextIdCounter is static and private, we can't directly reset it.
//     // Tests should use explicit IDs to avoid interdependencies.
// }

// ---------------------------
// Tests: Constructor & Destructor
// ---------------------------

TEST_CASE("WebLayout constructor with default ID", "[weblayout][constructor]") {
    WebLayout layout;
    std::string id = layout.Id();
    CHECK(!id.empty());
    CHECK(id.find("weblayout-") == 0); // Auto-generated ID
}

TEST_CASE("WebLayout constructor with explicit ID", "[weblayout][constructor]") {
    WebLayout layout("my-custom-layout");
    CHECK(layout.Id() == "my-custom-layout");
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout destructor is safe and idempotent", "[weblayout][destructor]") {
    {
        WebLayout layout("destruct-test");
        layout.MountToLayout(root);
    }
    // Destructor called here, should clean up DOM
    CHECK(GetElement("destruct-test") == val::undefined());
}

// ---------------------------
// Tests: IDomElement Interface
// ---------------------------

TEST_CASE_METHOD(SharedWebContext, "WebLayout implements IDomElement contract", "[weblayout][dom]") {
    STATIC_REQUIRE(std::is_base_of_v<IDomElement, WebLayout>);
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout ID remains stable after operations", "[weblayout][dom]") {
    WebLayout layout("stable-id-test");
    CHECK(layout.Id() == "stable-id-test");

    // ID should not change even if we mount/unmount
    layout.Unmount();
    CHECK(layout.Id() == "stable-id-test");
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout mountToLayout and unmount are safe", "[weblayout][dom]") {
    WebLayout parent("mount-parent");
    parent.MountToLayout(root);
    WebLayout child("mount-child");
    child.MountToLayout(parent, Alignment::Center);
    auto parent_elem = GetElement(parent.Id());
    auto child_elem = GetElement(child.Id());

    CHECK(GetProperty(child_elem, "parentElement") == parent_elem);
    child.Unmount();
    CHECK(GetProperty(child_elem, "parentElement") == val::undefined());
    child.Unmount(); // Idempotent unmount
    CHECK(GetProperty(child_elem, "parentElement") == val::undefined());
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout syncFromModel calls Apply", "[weblayout][dom]") {
    WebLayout layout("sync-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());
    layout.SetMargin(10);
    layout.SyncFromModel();
    CHECK(GetCSSProperty(elem, "margin") == "10px");
}

// ---------------------------
// Tests: Layout Configuration
// ---------------------------

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetLayoutType stores and applies correctly",
                 "[weblayout][layout-config]") {
    WebLayout layout("layout-type-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    layout.SetLayoutType(LayoutType::Horizontal);
    layout.Apply();

    CHECK(GetCSSProperty(elem, "display") == "flex");
    CHECK(GetCSSProperty(elem, "flex-direction") == "row");

    layout.SetLayoutType(LayoutType::Vertical);
    layout.Apply();

    CHECK(GetCSSProperty(elem, "display") == "flex");
    CHECK(GetCSSProperty(elem, "flex-direction") == "column");

    layout.SetLayoutType(LayoutType::Grid);
    layout.Apply();

    CHECK(GetCSSProperty(elem, "display") == "grid");

    layout.SetLayoutType(LayoutType::Free);
    layout.Apply();

    CHECK(GetCSSProperty(elem, "display") == "block");
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetJustification stores multiple values", "[weblayout][layout-config]") {
    WebLayout layout("justification-test");
    layout.SetLayoutType(LayoutType::Horizontal);
    layout.MountToLayout(root);
    MockDomElement element("justification-test-element");
    element.MountToLayout(layout);
    auto elem = GetElement(layout.Id());

    std::vector<Justification> justifications = {
            Justification::Start,       Justification::Center,      Justification::End, Justification::SpaceBetween,
            Justification::SpaceAround, Justification::SpaceEvenly, Justification::None};

    auto getJustifyStr = [](Justification j) -> string {
        switch (j) {
            case Justification::Start:
                return "flex-start";
            case Justification::Center:
                return "center";
            case Justification::End:
                return "flex-end";
            case Justification::SpaceBetween:
                return "space-between";
            case Justification::SpaceAround:
                return "space-around";
            case Justification::SpaceEvenly:
                return "space-evenly";
            case Justification::None:
                return "";
        }
    };

    for (auto j: justifications) {
        layout.SetJustification(j);
        layout.Apply();

        CHECK(GetCSSProperty(elem, "justify-content") == getJustifyStr(j));
    }
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetAlignItems stores alignment values", "[weblayout][layout-config]") {
    WebLayout layout("align-items-test");
    layout.SetLayoutType(LayoutType::Vertical);
    layout.MountToLayout(root);
    MockDomElement element("align-items-test-element");
    element.MountToLayout(layout);
    auto elem = GetElement(layout.Id());

    std::vector<Alignment> alignments = {Alignment::None, Alignment::Start, Alignment::Center, Alignment::End,
                                         Alignment::Stretch};

    auto getAlignItemsStr = [](Alignment a) -> std::string {
        switch (a) {
            case Alignment::Start:
                return "flex-start";
            case Alignment::Center:
                return "center";
            case Alignment::End:
                return "flex-end";
            case Alignment::Stretch:
                return "stretch";
            case Alignment::None:
                return "";
        }
    };

    for (auto a: alignments) {
        layout.SetAlignItems(a);
        layout.Apply();

        CHECK(GetCSSProperty(elem, "align-items") == getAlignItemsStr(a));
    }
}

// ---------------------------
// Tests: Child Element Management
// ---------------------------

TEST_CASE_METHOD(SharedWebContext, "WebLayout AddElement with null pointer returns false", "[weblayout][children]") {
    WebLayout layout("add-null-test");
    layout.MountToLayout(root);
    CHECK(layout.AddElement(nullptr) == false);
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout AddElement with invalid element ID returns false",
                 "[weblayout][children]") {
    WebLayout layout("add-invalid-test");
    layout.MountToLayout(root);
    MockDomElement elem(""); // Empty ID
    CHECK(layout.AddElement(&elem) == false);
    CHECK(elem.mountCount == 0);
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout can add and remove single element", "[weblayout][children]") {
    WebLayout layout("add-remove-test");
    layout.MountToLayout(root);
    MockDomElement elem("add-remove-elem");

    elem.MountToLayout(layout, Alignment::Center);
    layout.Apply();

    CHECK(layout.RemoveElement(&elem) == true);
    layout.Apply();
    CHECK(elem.mountCount == 1);
    CHECK(elem.unmountCount == 0);
    CHECK(elem.syncCount == 1);
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout RemoveElement returns false for non-existent element",
                 "[weblayout][children]") {
    WebLayout layout("remove-nonexist-test");
    layout.MountToLayout(root);
    MockDomElement elem("elem-1");

    CHECK(layout.RemoveElement(&elem) == false);
    CHECK(elem.unmountCount == 0);
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout RemoveElement with null pointer returns false", "[weblayout][children]") {
    WebLayout layout("remove-null-test");
    layout.MountToLayout(root);
    CHECK(layout.RemoveElement(nullptr) == false);
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetAlignment on non-existent element is safe", "[weblayout][children]") {
    WebLayout layout("set-align-test");
    layout.SetLayoutType(LayoutType::Horizontal);
    layout.MountToLayout(root);
    MockDomElement elem("elem-1");
    elem.MountToLayout(layout, Alignment::Center);
    auto elem_val = GetElement(elem.Id());

    layout.Apply();
    CHECK(GetCSSProperty(elem_val, "align-self") == "center");

    layout.SetAlignment(nullptr, Alignment::Center);
    layout.Apply();
    CHECK(GetCSSProperty(elem_val, "align-self") == "center");
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetAlignment on added element works", "[weblayout][children]") {
    WebLayout layout("set-align-added-test");
    layout.SetLayoutType(LayoutType::Horizontal);
    layout.MountToLayout(root);
    MockDomElement elem("elem-1");
    elem.MountToLayout(layout, Alignment::Start);
    auto elem_val = GetElement(elem.Id());

    layout.SetAlignment(&elem, Alignment::Center);
    layout.SetAlignment(&elem, Alignment::End);
    layout.Apply();
    CHECK(GetCSSProperty(elem_val, "align-self") == "flex-end");
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout can add multiple elements", "[weblayout][children]") {
    WebLayout layout("multiple-children-test");
    layout.MountToLayout(root);
    MockDomElement elem1("multiple-children-test-elem-1");
    MockDomElement elem2("multiple-children-test-elem-2");
    MockDomElement elem3("multiple-children-test-elem-3");

    elem1.MountToLayout(layout, Alignment::Start);
    elem2.MountToLayout(layout, Alignment::Center);
    elem3.MountToLayout(layout, Alignment::End);

    layout.Apply();

    // Remove first and verify others remain
    CHECK(layout.RemoveElement(&elem1) == true);
    layout.Apply();

    // Second should still be removable
    CHECK(layout.RemoveElement(&elem2) == true);
    CHECK(layout.RemoveElement(&elem3) == true);

    CHECK(elem1.mountCount == 1);
    CHECK(elem2.mountCount == 1);
    CHECK(elem3.mountCount == 1);
    CHECK(elem1.unmountCount == 0);
    CHECK(elem2.unmountCount == 0);
    CHECK(elem3.unmountCount == 0);
    CHECK(elem1.syncCount == 1);
    CHECK(elem2.syncCount == 2);
    CHECK(elem3.syncCount == 2);
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout Clear removes all elements", "[weblayout][children]") {
    WebLayout layout("clear-test");
    layout.MountToLayout(root);
    MockDomElement elem1("clear-test-elem-1");
    MockDomElement elem2("clear-test-elem-2");

    elem1.MountToLayout(layout);
    elem2.MountToLayout(layout);

    layout.Clear();

    // After clear, elements should not be removable (not in layout)
    CHECK(layout.RemoveElement(&elem1) == false);
    CHECK(layout.RemoveElement(&elem2) == false);
    CHECK(elem1.mountCount == 1);
    CHECK(elem2.mountCount == 1);
    CHECK(elem1.unmountCount == 1);
    CHECK(elem2.unmountCount == 1);
}

// ---------------------------
// Tests: Styling Methods
// ---------------------------

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetSpacing stores positive values", "[weblayout][styling]") {
    WebLayout layout("spacing-test");
    layout.SetLayoutType(LayoutType::Horizontal);
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    layout.SetSpacing(0);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "gap") == "0px");

    layout.SetSpacing(10);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "gap") == "10px");

    layout.SetSpacing(-10);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "gap") == "10px");
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetBackgroundColor stores colors", "[weblayout][styling]") {
    WebLayout layout("bg-color-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    std::vector<std::string> colors{"#ffffff",     "#000000", "red", "rgb(255, 0, 0)", "rgba(255, 0, 0, 0.5)",
                                    "transparent", ""};

    std::vector<std::string> outputs{"rgb(255, 255, 255)",   "rgb(0, 0, 0)", "red", "rgb(255, 0, 0)",
                                     "rgba(255, 0, 0, 0.5)", "transparent",  ""};

    for (int i{0}; i < colors.size(); i++) {
        layout.SetBackgroundColor(colors[i]);
        layout.Apply();
        CHECK(GetCSSProperty(elem, "background-color") == outputs[i]);
    }
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetBorderColor stores colors", "[weblayout][styling]") {
    WebLayout layout("border-color-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    layout.SetBorderColor("#ff0000");
    layout.Apply();
    CHECK(GetCSSProperty(elem, "border-color") == "rgb(255, 0, 0)");

    layout.SetBorderColor("blue");
    layout.Apply();
    CHECK(GetCSSProperty(elem, "border-color") == "blue");

    layout.SetBorderColor("");
    layout.Apply();
    CHECK(GetCSSProperty(elem, "border-color") == "");
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetBorderWidth stores positive values", "[weblayout][styling]") {
    WebLayout layout("border-width-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    std::vector<int> widths = {0, 1, 5, 10, 100, -10};
    for (int w: widths) {
        layout.SetBorderWidth(w);
        layout.Apply();

        if (w >= 0)
            CHECK(GetCSSProperty(elem, "border-width") == std::to_string(w) + "px");
        else
            CHECK(GetCSSProperty(elem, "border-width") != std::to_string(w) + "px");
    }
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetBorderRadius stores positive values", "[weblayout][styling]") {
    WebLayout layout("border-radius-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    std::vector<int> radii = {0, 1, 5, 10, 50, -10};
    for (int r: radii) {
        layout.SetBorderRadius(r);
        layout.Apply();
        if (r >= 0)
            CHECK(GetCSSProperty(elem, "border-radius") == std::to_string(r) + "px");
        else
            CHECK(GetCSSProperty(elem, "border-radius") != std::to_string(r) + "px");
    }
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetPadding stores positive values", "[weblayout][styling]") {
    WebLayout layout("padding-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    layout.SetPadding(20);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "padding") == "20px");

    layout.SetPadding(0);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "padding") == "0px");

    layout.SetPadding(-10);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "padding") == "0px");
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetMargin stores positive values", "[weblayout][styling]") {
    WebLayout layout("margin-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    layout.SetMargin(20);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "margin") == "20px");

    layout.SetMargin(0);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "margin") == "0px");

    layout.SetMargin(-10);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "margin") == "0px");
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetWidth stores positive values", "[weblayout][styling]") {
    WebLayout layout("width-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    layout.SetWidth(100);
    layout.Apply();
    REQUIRE(GetCSSProperty(elem, "width") == "100px");

    layout.SetWidth(1000);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "width") == "1000px");

    layout.SetWidth(-10);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "width") == "1000px");
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetHeight stores positive values", "[weblayout][styling]") {
    WebLayout layout("height-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    layout.SetHeight(100);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "height") == "100px");

    layout.SetHeight(1000);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "height") == "1000px");

    layout.SetHeight(-10);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "height") == "1000px");
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetOpacity stores values in valid range", "[weblayout][styling]") {
    WebLayout layout("opacity-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    layout.SetOpacity(0.0);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "opacity") == "0");

    layout.SetOpacity(0.5);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "opacity") == "0.5");

    layout.SetOpacity(-0.5);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "opacity") == "0.5");
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetBoxShadow stores shadow strings", "[weblayout][styling]") {
    WebLayout layout("box-shadow-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    std::vector<std::string> shadows = {"rgba(0, 0, 0, 0.1) 0px 4px 6px", "rgb(0, 0, 0) 0px 0px 10px",
                                        "rgb(204, 204, 204) 0px 0px 5px inset", ""};

    for (const auto& shadow: shadows) {
        layout.SetBoxShadow(shadow);
        layout.Apply();
        CHECK(GetCSSProperty(elem, "box-shadow") == shadow);
    }
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout ToggleVisibility toggles state", "[weblayout][visibility]") {
    WebLayout layout("visibility-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    layout.ToggleVisibility();
    layout.Apply();
    CHECK(GetCSSProperty(elem, "display") == "none");

    layout.ToggleVisibility();
    layout.Apply();
    CHECK(GetCSSProperty(elem, "display") == "block");
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout ToggleVisibility prevents style application",
                 "[weblayout][styling][visibility]") {
    WebLayout layout("visibility-multiple-test");
    layout.MountToLayout(root);
    MockDomElement elem("visibility-multiple-test-elem");

    elem.MountToLayout(layout);
    layout.ToggleVisibility();

    layout.Apply();
    CHECK(elem.syncCount == 0);
}

// ---------------------------
// Tests: Combined Styling
// ---------------------------

TEST_CASE_METHOD(SharedWebContext, "WebLayout complex styling scenario", "[weblayout][styling][integration]") {
    WebLayout layout("complex-style-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

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
    layout.SetBoxShadow("rgba(0, 0, 0, 0.1) 0 4px 6px");

    layout.Apply();

    CHECK(GetCSSProperty(elem, "background-color") == "lightblue");
    CHECK(GetCSSProperty(elem, "border-color") == "navy");
    CHECK(GetCSSProperty(elem, "border-width") == "2px");
    CHECK(GetCSSProperty(elem, "border-radius") == "8px");
    CHECK(GetCSSProperty(elem, "padding") == "10px");
    CHECK(GetCSSProperty(elem, "margin") == "5px");
    CHECK(GetCSSProperty(elem, "width") == "300px");
    CHECK(GetCSSProperty(elem, "height") == "200px");
    CHECK(GetCSSProperty(elem, "opacity") == "0.95");
    CHECK(GetCSSProperty(elem, "box-shadow") == "rgba(0, 0, 0, 0.1) 0px 4px 6px");
}

// ---------------------------
// Tests: Grid Positioning
// ---------------------------

TEST_CASE_METHOD(SharedWebContext, "IDomElement grid position helpers work correctly",
                 "[weblayout][positioning][grid]") {
    MockDomElement elem("grid-test");

    // Default state: unset (-1)
    CHECK(elem.GridRow() == -1);
    CHECK(elem.GridCol() == -1);

    // Set positions
    elem.SetGridPosition(2, 3);
    CHECK(elem.GridRow() == 2);
    CHECK(elem.GridCol() == 3);

    // Change positions
    elem.SetGridPosition(0, 0);
    CHECK(elem.GridRow() == 0);
    CHECK(elem.GridCol() == 0);

    // Clear positions
    elem.ClearGridPosition();
    CHECK(elem.GridRow() == -1);
    CHECK(elem.GridCol() == -1);
}

TEST_CASE_METHOD(SharedWebContext, "IDomElement grid position with large indices", "[weblayout][positioning][grid]") {
    MockDomElement elem("large-grid-test");

    elem.SetGridPosition(100, 200);
    CHECK(elem.GridRow() == 100);
    CHECK(elem.GridCol() == 200);
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout applies grid positions to children", "[weblayout][positioning][grid]") {
    WebLayout layout("grid-layout-test");
    layout.SetLayoutType(LayoutType::Grid);
    layout.MountToLayout(root);

    MockDomElement elem("grid-child");
    elem.SetGridPosition(1, 2);

    elem.MountToLayout(layout);
    layout.Apply();

    // Grid position should be preserved
    CHECK(elem.GridRow() == 1);
    CHECK(elem.GridCol() == 2);
}

// ---------------------------
// Tests: Free Layout Positioning
// ---------------------------

TEST_CASE_METHOD(SharedWebContext, "IDomElement free position helpers work correctly",
                 "[weblayout][positioning][free]") {
    MockDomElement elem("free-test");

    // Default state: unset (-1)
    CHECK(elem.FreeTop() == -1);
    CHECK(elem.FreeLeft() == -1);

    // Set positions
    elem.SetFreePosition(50, 100);
    CHECK(elem.FreeTop() == 50);
    CHECK(elem.FreeLeft() == 100);

    // Change positions
    elem.SetFreePosition(10, 20);
    CHECK(elem.FreeTop() == 10);
    CHECK(elem.FreeLeft() == 20);

    // Clear positions
    elem.ClearFreePosition();
    CHECK(elem.FreeTop() == -1);
    CHECK(elem.FreeLeft() == -1);
}

TEST_CASE_METHOD(SharedWebContext, "IDomElement free position with zero coordinates",
                 "[weblayout][positioning][free]") {
    MockDomElement elem("zero-free-test");

    elem.SetFreePosition(0, 0);
    CHECK(elem.FreeTop() == 0);
    CHECK(elem.FreeLeft() == 0);
}

TEST_CASE_METHOD(SharedWebContext, "IDomElement free position with large coordinates",
                 "[weblayout][positioning][free]") {
    MockDomElement elem("large-free-test");

    elem.SetFreePosition(5000, 10000);
    CHECK(elem.FreeTop() == 5000);
    CHECK(elem.FreeLeft() == 10000);
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout applies free positions to children", "[weblayout][positioning][free]") {
    WebLayout layout("free-layout-test");
    layout.SetLayoutType(LayoutType::Free);
    layout.MountToLayout(root);

    MockDomElement elem("free-child");
    elem.SetFreePosition(50, 75);

    elem.MountToLayout(layout);
    layout.Apply();

    // Free position should be preserved
    CHECK(elem.FreeTop() == 50);
    CHECK(elem.FreeLeft() == 75);
}

// ---------------------------
// Tests: Edge Cases
// ---------------------------

TEST_CASE_METHOD(SharedWebContext, "WebLayout with empty ID is safe", "[weblayout][edge-case]") {
    WebLayout layout("");
    // Constructor should handle empty string by generating an ID
    CHECK(!layout.Id().empty());
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout remove same element multiple times", "[weblayout][edge-case]") {
    WebLayout layout("add-remove-repeat-test");
    layout.MountToLayout(root);
    MockDomElement elem("repeat-elem");

    elem.MountToLayout(layout);
    CHECK(layout.RemoveElement(&elem) == true);
    CHECK(layout.RemoveElement(&elem) == false); // Already removed
    CHECK(elem.mountCount == 1);
    CHECK(elem.unmountCount == 0);
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout SetAlignment on untrack element is safe", "[weblayout][edge-case]") {
    WebLayout layout("untrack-align-test");
    layout.MountToLayout(root);
    MockDomElement elem1("untrack-align-test-elem-1");
    MockDomElement elem2("untrack-align-test-elem-2");

    elem1.MountToLayout(layout);
    layout.SetAlignment(&elem2, Alignment::Center); // Not tracked
    CHECK(elem1.mountCount == 1);
    CHECK(elem2.mountCount == 0);
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout consecutive Clear calls are safe", "[weblayout][edge-case]") {
    WebLayout layout("clear-clear-test");
    layout.MountToLayout(root);
    MockDomElement elem("clear-clear-test-elem");
    elem.MountToLayout(layout);
    auto elem_val = GetElement(elem.Id());

    layout.Clear();
    layout.Clear(); // Should be safe
    CHECK(GetProperty(elem_val, "parentElement") == val::undefined());
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout nested layouts", "[weblayout][edge-case]") {
    WebLayout parent("nested-parent");
    parent.MountToLayout(root);
    WebLayout child("nested-child");
    MockDomElement grandchild("nested-grandchild");

    child.MountToLayout(parent);
    grandchild.MountToLayout(child);
    parent.Apply();
    CHECK(grandchild.mountCount == 1);
    CHECK(grandchild.syncCount == 1);
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout very large spacing values", "[weblayout][edge-case]") {
    WebLayout layout("large-spacing-test");
    layout.SetLayoutType(LayoutType::Vertical);
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    layout.SetSpacing(10000);
    layout.SetPadding(10000);
    layout.SetMargin(10000);
    layout.SetWidth(10000);
    layout.SetHeight(10000);
    layout.Apply();
    CHECK(GetCSSProperty(elem, "gap") == "10000px");
    CHECK(GetCSSProperty(elem, "padding") == "10000px");
    CHECK(GetCSSProperty(elem, "margin") == "10000px");
    CHECK(GetCSSProperty(elem, "width") == "10000px");
    CHECK(GetCSSProperty(elem, "height") == "10000px");
}

// ---------------------------
// Tests: State Preservation
// ---------------------------

TEST_CASE_METHOD(SharedWebContext, "WebLayout preserves styling through multiple Apply calls", "[weblayout][state]") {
    WebLayout layout("state-preservation-test");
    layout.MountToLayout(root);
    auto elem = GetElement(layout.Id());

    layout.SetBackgroundColor("red");
    layout.SetBorderWidth(3);
    layout.SetPadding(10);

    layout.Apply();

    CHECK(GetCSSProperty(elem, "background-color") == "red");
    CHECK(GetCSSProperty(elem, "border-width") == "3px");
    CHECK(GetCSSProperty(elem, "padding") == "10px");

    layout.Apply();
    layout.Apply();

    CHECK(GetCSSProperty(elem, "background-color") == "red");
    CHECK(GetCSSProperty(elem, "border-width") == "3px");
    CHECK(GetCSSProperty(elem, "padding") == "10px");

    // Should still be applied
}

TEST_CASE_METHOD(SharedWebContext, "WebLayout preserves child order through operations", "[weblayout][state]") {
    WebLayout layout("child-order-test");
    layout.MountToLayout(root);
    MockDomElement elem1("child-order-test-elem-1");
    MockDomElement elem2("child-order-test-elem-2");
    MockDomElement elem3("child-order-test-elem-3");
    elem1.MountToLayout(layout);
    elem2.MountToLayout(layout);
    elem3.MountToLayout(layout);
    auto layout_val = GetElement(layout.Id());
    auto val_1 = GetElement(elem1.Id());
    auto val_2 = GetElement(elem2.Id());
    auto val_3 = GetElement(elem3.Id());


    layout.Apply();

    auto firstChildId = GetProperty(layout_val, "firstElementChild")["id"].as<string>();
    auto lastChildId = GetProperty(layout_val, "lastElementChild")["id"].as<string>();

    layout.Apply(); // Order should be preserved

    auto appliedFirstChildId = GetProperty(layout_val, "firstElementChild")["id"].as<string>();
    auto appliedLastChildId = GetProperty(layout_val, "lastElementChild")["id"].as<string>();

    CHECK(appliedFirstChildId == firstChildId);
    CHECK(appliedLastChildId == lastChildId);

    // Now remove middle element
    layout.RemoveElement(&elem2);
    layout.Apply();

    auto removalFirstChildId = GetProperty(layout_val, "firstElementChild")["id"].as<string>();
    auto removalLastChildId = GetProperty(layout_val, "lastElementChild")["id"].as<string>();

    CHECK(removalFirstChildId == firstChildId);
    CHECK(removalLastChildId == lastChildId);

    CHECK(elem1.mountCount == 1);
    CHECK(elem2.mountCount == 1);
    CHECK(elem3.mountCount == 1);
    CHECK(elem1.syncCount == 3);
    CHECK(elem2.syncCount == 2);
    CHECK(elem3.syncCount == 3);
    CHECK(elem2.unmountCount == 0);
}

// ---------------------------
// Additional Tests
// ---------------------------

// ========================================================
// Adding the same element twice does not duplicate it
// ========================================================
TEST_CASE_METHOD(SharedWebContext, "WebLayout does not duplicate child when added twice",
                 "[weblayout][children][dom]") {
    WebLayout layout("duplicate-add-test");
    layout.MountToLayout(root);

    MockDomElement elem("duplicate-add-child");
    elem.MountToLayout(layout);
    elem.MountToLayout(layout); // same child again
    layout.Apply();

    val layoutElem = GetElement(layout.Id());
    CHECK(GetProperty(layoutElem, "children")["length"].as<int>() == 1);
    CHECK(elem.mountCount == 2); // AddElement returned true both times
    CHECK(elem.syncCount == 1); // but only tracked/applied once
}

// ========================================================
// Grid layout applies row and column CSS to child
// ========================================================
TEST_CASE_METHOD(SharedWebContext, "WebLayout applies grid CSS positions to child",
                 "[weblayout][positioning][grid][dom]") {
    WebLayout layout("grid-css-test");
    layout.SetLayoutType(LayoutType::Grid);
    layout.MountToLayout(root);

    MockDomElement elem("grid-css-child");
    elem.SetGridPosition(2, 3);
    elem.MountToLayout(layout);

    layout.Apply();

    val elemVal = GetElement(elem.Id());
    CHECK(GetCSSProperty(elemVal, "grid-row-start") == "3");
    CHECK(GetCSSProperty(elemVal, "grid-column-start") == "4");
}

// ========================================================
// Free layout applies top and left CSS to child
// ========================================================
TEST_CASE_METHOD(SharedWebContext, "WebLayout applies free layout CSS positions to child",
                 "[weblayout][positioning][free][dom]") {
    WebLayout layout("free-css-test");
    layout.SetLayoutType(LayoutType::Free);
    layout.MountToLayout(root);

    MockDomElement elem("free-css-child");
    elem.SetFreePosition(40, 70);
    elem.MountToLayout(layout);

    layout.Apply();

    val elemVal = GetElement(elem.Id());
    CHECK(GetCSSProperty(elemVal, "top") == "40px");
    CHECK(GetCSSProperty(elemVal, "left") == "70px");
    CHECK(GetCSSProperty(elemVal, "position") == "relative");
}

// ========================================================
// Hidden layout becomes visible again with correct display
// ========================================================
TEST_CASE_METHOD(SharedWebContext, "WebLayout can hide and show repeatedly", "[weblayout][visibility][dom]") {
    WebLayout layout("hide-show-repeat-test");
    layout.SetLayoutType(LayoutType::Vertical);
    layout.MountToLayout(root);

    val elem = GetElement(layout.Id());

    layout.Apply();
    CHECK(GetCSSProperty(elem, "display") == "flex");

    layout.ToggleVisibility();
    layout.Apply();
    CHECK(GetCSSProperty(elem, "display") == "none");

    layout.ToggleVisibility();
    layout.Apply();
    CHECK(GetCSSProperty(elem, "display") == "flex");
}

// ========================================================
// Child sync resumes after layout becomes visible again
// ========================================================
TEST_CASE_METHOD(SharedWebContext, "Child sync resumes after layout is shown again",
                 "[weblayout][visibility][children]") {
    WebLayout layout("sync-resume-test");
    layout.MountToLayout(root);

    MockDomElement elem("sync-resume-child");
    elem.MountToLayout(layout);

    layout.Apply();
    CHECK(elem.syncCount == 1);

    layout.ToggleVisibility(); // hidden
    layout.Apply();
    CHECK(elem.syncCount == 1);

    layout.ToggleVisibility(); // visible again
    layout.Apply();
    CHECK(elem.syncCount == 2);
}

// ========================================================
// Styling set before mount persists after mount and apply
// ========================================================
TEST_CASE_METHOD(SharedWebContext, "WebLayout pre-mount styling persists after mount", "[weblayout][styling][dom]") {
    WebLayout layout("premount-style-test");
    layout.SetBackgroundColor("red");
    layout.SetBorderWidth(2);
    layout.SetPadding(12);
    layout.SetMargin(8);
    layout.SetWidth(250);
    layout.SetHeight(150);

    layout.MountToLayout(root);
    layout.Apply();

    val elem = GetElement(layout.Id());
    CHECK(GetCSSProperty(elem, "background-color") == "red");
    CHECK(GetCSSProperty(elem, "border-width") == "2px");
    CHECK(GetCSSProperty(elem, "padding") == "12px");
    CHECK(GetCSSProperty(elem, "margin") == "8px");
    CHECK(GetCSSProperty(elem, "width") == "250px");
    CHECK(GetCSSProperty(elem, "height") == "150px");
}

// ========================================================
// Child can be moved from one layout to another
// ========================================================
TEST_CASE_METHOD(SharedWebContext, "Child can be remounted from one layout to another", "[weblayout][children][dom]") {
    WebLayout layout1("remount-parent-1");
    WebLayout layout2("remount-parent-2");
    layout1.MountToLayout(root);
    layout2.MountToLayout(root);

    MockDomElement elem("remount-child");
    elem.MountToLayout(layout1);
    layout1.Apply();
    layout2.Apply();

    val parent1 = GetElement(layout1.Id());
    val parent2 = GetElement(layout2.Id());

    CHECK(GetProperty(parent1, "children")["length"].as<int>() == 1);
    CHECK(GetProperty(parent2, "children")["length"].as<int>() == 0);

    elem.MountToLayout(layout2);
    layout1.Apply();
    layout2.Apply();

    CHECK(GetProperty(parent2, "children")["length"].as<int>() == 1);
    CHECK(GetElement(elem.Id())["parentElement"]["id"].as<std::string>() == layout2.Id());
}

// ========================================================
// SetAlignment applies all alignment values correctly
// ========================================================
TEST_CASE_METHOD(SharedWebContext, "WebLayout SetAlignment applies start center end stretch",
                 "[weblayout][children][style]") {
    WebLayout layout("child-align-all-test");
    layout.SetLayoutType(LayoutType::Horizontal);
    layout.MountToLayout(root);

    MockDomElement elem("child-align-all");
    elem.MountToLayout(layout);
    val elemVal = GetElement(elem.Id());

    layout.SetAlignment(&elem, Alignment::Start);
    layout.Apply();
    CHECK(GetCSSProperty(elemVal, "align-self") == "flex-start");

    layout.SetAlignment(&elem, Alignment::Center);
    layout.Apply();
    CHECK(GetCSSProperty(elemVal, "align-self") == "center");

    layout.SetAlignment(&elem, Alignment::End);
    layout.Apply();
    CHECK(GetCSSProperty(elemVal, "align-self") == "flex-end");

    layout.SetAlignment(&elem, Alignment::Stretch);
    layout.Apply();
    CHECK(GetCSSProperty(elemVal, "align-self") == "stretch");
}

// ========================================================
// Clear removes all child DOM nodes from layout element
// ========================================================
TEST_CASE_METHOD(SharedWebContext, "WebLayout Clear removes child DOM nodes", "[weblayout][children][dom]") {
    WebLayout layout("clear-dom-test");
    layout.MountToLayout(root);

    MockDomElement elem1("clear-dom-child-1");
    MockDomElement elem2("clear-dom-child-2");

    elem1.MountToLayout(layout);
    elem2.MountToLayout(layout);
    layout.Apply();

    val layoutElem = GetElement(layout.Id());
    CHECK(GetProperty(layoutElem, "children")["length"].as<int>() == 2);

    layout.Clear();

    CHECK(GetProperty(layoutElem, "children")["length"].as<int>() == 0);
}

// ========================================================
// Apply on empty layout is safe
// ========================================================
TEST_CASE_METHOD(SharedWebContext, "WebLayout Apply on empty layout is safe", "[weblayout][edge-case]") {
    WebLayout layout("empty-apply-test");
    layout.MountToLayout(root);

    REQUIRE_NOTHROW(layout.Apply());
    val elem = GetElement(layout.Id());
    CHECK(!elem.isNull());
    CHECK(!elem.isUndefined());
}

#endif
