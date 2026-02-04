> [!NOTE]
>
> This document elaborates Section 3.2 of the README.
> Section numbers here follow the global numbering (3.2.x).

# Group 18's Write-up for Initial C++ Class (WebUI)

*Last updated: Jan 30, 2026*

## 3.2 `WebTextbox` Class Specification

---

### 3.2.1 Class Description

A C++ class that represents a logical text box used in the Web UI; in our implementation it will eventually be rendered as an HTML element (such as `<p>` and `<span>`) via `WebLayout`, or a canvas element via `WebCanvas`. Supports setting text content, font properties, alignment, color, size constraints, and visibility. Intended for HUD displays, dialogue boxes, logs, tooltips, and debug overlays.  
`WebTextbox` serves as the Web-GUI implementation of the shared text interface used across groups, ensuring it is interchangeable with Group 17’s local GUI Text class.

---

### 3.2.2 Similar Standard Library Classes

- **`std::string`** – for storing and formatting text  
- **`std::ostringstream`** – for constructing dynamic text output  
- **Emscripten’s `val`** – for JavaScript/DOM interaction  

---

### 3.2.3 Key Functions

```cpp
// Construction
WebTextbox(const std::string& elementId);
~WebTextbox();

// Core Text Operations
void SetText(const std::string& text);
void AppendText(const std::string& text);
std::string GetText() const;
void Clear();

// Formatting
void SetFontFamily(const std::string& family);
void SetFontSize(float sizePx);
void SetColor(const std::string& cssColor);
void SetBold(bool enabled);
void SetItalic(bool enabled);
void SetAlignment(const std::string& alignment); // "left", "center", "right"

// Layout
void SetPosition(int x, int y);
void SetMaxWidth(float widthPx);

// Visibility
void Show();
void Hide();
bool IsVisible() const;
```

---

### 3.2.4 Error Conditions

| Type | Example | Handling |
|------|---------|----------|
| Programmer error | Invalid DOM ID, negative font size | assert() |
| Recoverable error | DOM update failure, missing element at runtime | Return false + console debug log |
| User error | Invalid input (if textbox later supports editing) | Ignore or return special state |

Following instructor guidance, **no C++ exceptions** will be thrown inside `WebTextbox` for WebAssembly builds.

---

### 3.2.5 Expected Challenges

- Understanding Emscripten C++ ↔ JavaScript bridging   
- Ensuring efficient DOM updates to avoid layout performance issues  
- Integrating text elements cleanly within WebLayout  
- Handling word-wrapping, adaptive sizing, and multi-line UI elements  
- Debugging errors across C++ and JavaScript without exception support  

---

### 3.2.6 Coordination with Other Classes

| Class | Group | Reason |
|--------|--------|---------|
| **`Text`** | 17 (GUI Interface) | Must match API to ensure GUI interchangeability |
| **`Menu`** | 17 | Shared text formatting conventions for menu labels |
| **`WebLayout`** | 18 | Controls positioning and DOM structure for textboxes |
| **`WebButton`** | 18 | Ensures UI styling consistency across widgets |
| **`ActionMap`** | 2 (Classic Agents) | Enables text-linked actions or UI command triggers |
| **`OutputManager`** | 16 (Data Analytics) | Supports log/text-output to debug consoles or HUD elements |
