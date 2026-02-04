> [!NOTE]
>
> This document elaborates Section 3.5 of the README.
> Section numbers here follow the global numbering (3.5.x).

# Group 18's Write-up for Initial C++ Class (WebUI)

*Last updated: Jan 30, 2026*

## 3.5 `WebLayout` Class Specification

---

### 3.5.1 Class Description

`WebLayout` manages the spatial arrangement and hierarchy of web-based UI elements (e.g., `WebImage`, `WebButton`, `WebTextbox`, `WebCanvas`) within the DOM. Its goal is to provide a simple C++ interface to position, align, group, and update HTML elements without requiring the programmer to manually write JavaScript or manipulate the DOM directly.

High-level functionality includes:

- Creating layout containers (horizontal, vertical, grid, or free-positioned).
- Adding/removing child elements.
- Controlling spacing, alignment, and sizing rules.
- Updating DOM structure efficiently when the layout changes.

---

### 3.5.2 Relevant Standard Library Analogues

These classes are not direct equivalents but provide conceptual parallels:

- `std::vector` — ordered container behavior for child elements.
- `std::map` / `std::unordered_map` — mapping element IDs to layout metadata.
- `std::function` — callbacks for dynamic layout updates.

---

### 3.5.3 Planned Key Functions

- `AddElement(ElementID id, LayoutParams params)`
- `RemoveElement(ElementID id)`
- `SetAlignment(ElementID id, Alignment a)`
- `SetSpacing(int px)`
- `SetLayoutType(LayoutType type)`
- `Apply()` — pushes layout changes to the DOM.
- `Clear()` — removes all elements from the layout.

---

### 3.5.4 Error Conditions

| Condition                                                    | Type             | Handling                       |
| ------------------------------------------------------------ | ---------------- | ------------------------------ |
| Invalid element ID passed to `AddElement`  or `RemoveElement` | Programmer error | `assert`                       |
| Layout parameters out of valid range (e.g., negative spacing) | Programmer error | `assert`                       |
| User provides invalid input (e.g., element not yet created)  | User error       | Return failure code or boolean |

---

### 3.5.5 Expected Challenges & Topics to Learn

- Understanding Emscripten’s interface with JavaScript and the DOM.
- Efficiently batching DOM updates to avoid performance bottlenecks.
- Designing layout abstractions that remain flexible for different game UIs.
- Coordinating with other Web Interface classes so that layout integrates cleanly with rendering and event handling.

---

### 3.5.6 Coordination With Other Groups

Likely dependencies and coordination points:

- **GUI Interface (Group 17)** — aligning interface expectations for layout behavior.
- **Agent API (Groups 1 & 2)** — ensuring layout can display agent-related UI elements (health bars, stats, etc.).
- **World modules (Groups 14 & 15)** — supporting dynamic world views rendered in a canvas.
