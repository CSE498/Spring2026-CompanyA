> [!NOTE]
>
> This document elaborates Section 3.2 of the README.  
> Section numbers here follow the global numbering (3.2.x).

# Group 18's Write-up for Initial C++ Class (WebUI)

*Last updated: April 2026*

## 3.2 `WebTextbox` Class Specification

---

### 3.2.1 Class Description

A C++ class that represents a styled, DOM-backed text element for the Web UI. Internally, it wraps an HTML `<div>` element and uses Emscripten (`emscripten::val`) to synchronize C++ state with browser DOM properties.  

`WebTextbox` supports rich text styling (font family, size, bold/italic, alignment), layout constraints (max width, wrapping), background color, visibility, and bounding box queries. It can be rendered in two ways:

- **DOM Mode (primary):** Mounted into a `WebLayout` container as part of the UI hierarchy  
- **Canvas Mode (secondary):** Drawn directly onto a `WebCanvas` using the same text/style properties  

This dual-interface design allows `WebTextbox` to function as both a **layout element (`IDomElement`)** and a **renderable object (`ICanvasElement`)**.  

`WebTextbox` serves as the Web-GUI implementation of the shared text interface used across groups, ensuring compatibility with Group 17’s GUI system.

---

### 3.2.2 Similar Standard Library Classes

- **`std::string`** – stores and manipulates text content  
- **`std::optional`** – used for optional background color state  
- **`std::move` / value semantics** – used in move constructor/assignment  
- **Emscripten `val`** – bridges C++ with JavaScript DOM  

---

### 3.2.3 Key Functions

```cpp
// Construction / Move semantics
WebTextbox();
explicit WebTextbox(const std::string& initial_text);
WebTextbox(WebTextbox&& other) noexcept;
WebTextbox& operator=(WebTextbox&& other) noexcept;
~WebTextbox();

// Core Text Operations
void SetText(const std::string& text);
void AppendText(const std::string& text);
std::string GetText() const;
void Clear();

// Formatting / Style
void SetFontFamily(const std::string& family);
void SetFallbackFontFamily(const std::string& fallback_family);
void SetFontSize(float sizePx);
void SetLineHeight(float lineHeightPx);
void SetColor(const std::string& cssColor);
void SetBold(bool enabled);
void SetItalic(bool enabled);
void SetAlignment(TextAlign alignment); // enum: Left, Center, Right

// Layout / Behavior
void SetMaxWidth(float widthPx);
void SetWrap(bool enabled);
void SetBackgroundColor(const std::string& cssColor);
void ClearBackgroundColor();

// Bounding Box
RectPx GetBoundingBoxPx() const;
double GetWidthPx() const;
double GetHeightPx() const;

// Visibility
void Show();
void Hide();
bool IsVisible() const;

// DOM Integration
void MountToLayout(WebLayout& parent, Alignment align = Alignment::None);
void SyncFromModel();

// Canvas Integration
void SetCanvasPosition(float x, float y);
void Draw(WebCanvas& canvas);
```
---

### 3.2.4 Error Conditions

| Type | Example | Handling |
|------|---------|----------|
| Programmer error | Invalid enum, negative font size, invalid width | `assert()` + safe fallback |
| Recoverable error | DOM element not mounted, missing parent | Return default values (e.g., empty bounding box) |
| Runtime/UI issue | Requested font not available in browser | Console warning logged via JS |
| User error | Invalid CSS color or input | Passed through to browser (graceful degradation) |

Following project constraints, **no C++ exceptions are thrown** (WebAssembly-safe design).

---

### 3.2.5 Expected Challenges

- Managing **C++ ↔ JavaScript DOM synchronization** using Emscripten  
- Ensuring efficient updates (avoiding excessive `ApplyStyles()` calls)  
- Handling **layout vs. content separation** with `WebLayout`  
- Supporting both **DOM rendering and Canvas rendering paths** cleanly  
- Debugging browser-side issues without traditional exception handling  
- Ensuring consistent behavior across different browsers (fonts, layout, wrapping)  

---

### 3.2.6 Coordination with Other Classes

| Class | Group | Reason |
|--------|--------|---------|
| **`Text`** | 17 (GUI Interface) | Must maintain API compatibility for interchangeable UI systems |
| **`WebLayout`** | 18 | Responsible for mounting, positioning, and layout alignment |
| **`WebCanvas`** | 18 | Enables alternate rendering path for text drawing |
| **`WebButton`** | 18 | Ensures UI styling consistency |
| **`IDomElement`** | 18 | Interface for DOM-based UI elements |
| **`ICanvasElement`** | 18 | Interface for canvas-renderable elements |
| **World / Agent Systems** | Other groups | Provide dynamic text (HUD, logs, state updates) |

---
