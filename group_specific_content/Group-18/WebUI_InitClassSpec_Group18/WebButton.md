> [!NOTE]
>
> This document elaborates Section 3.3 of the README.
> Section numbers here follow the global numbering (3.3.x).

# Group 18's Write-up for Initial C++ Class (WebUI)

*Last updated: Jan 30, 2026*

## 3.3 `WebButton` Class Specification

---

### 3.3.1 Class Description

A C++ class that represents a logical button used in the Web UI; in our implementation it will eventually be rendered as an HTML element via `WebLayout`, or a canvas item via `WebCanvas`. Allows setting text, size, position, colors, and a callback function that executes when clicked. Supports enabling/disabling and showing/hiding the button.

---

### 3.3.2 Similar Standard Library Classes

- **`std::function`** - for storing the click callback
- **`std::string`** - for storing label text
- **Emscripten's `val`** - for JavaScript/DOM interaction

---

### 3.3.3 Key Functions

```cpp
// Construction
WebButton(const std::string& label);
~WebButton();

// Label
void SetLabel(const std::string& text);
std::string GetLabel() const;

// Callback
void SetCallback(std::function<void()> callback);
void Click();  // programmatic trigger

// Size & Position
void SetSize(int width, int height);
void SetPosition(int x, int y);

// Styling
void SetBackgroundColor(const std::string& color);
void SetTextColor(const std::string& color);

// State
void Enable();
void Disable();
bool IsEnabled() const;
void Show();
void Hide();
bool IsVisible() const;
```

---

### 3.3.4 Error Conditions

| Type | Example | Handling |
|------|---------|----------|
| Programmer error | Negative width/height, null callback | `assert()` |
| Recoverable error | DOM element creation fails | `std::runtime_error` |
| User error | Clicking disabled button | Ignore silently |

---

### 3.3.5 Expected Challenges

- Learning Emscripten's C++ to JavaScript bridging (embind, val)
- DOM manipulation from C++
- Memory cleanup (removing DOM elements, unbinding listeners)

---

### 3.3.6 Coordination with Other Classes

| Class | Group | Reason |
|-------|-------|--------|
| `WebLayout` | 18 | Manages button positioning |
| `WebTextbox` | 18 | Shared text styling |
| `ActionMap` | 2 (Classic Agents) | Similar string-to-function mapping pattern |
| `Menu` | 17 (GUI Interface) | Common interface for both GUI systems |
| `OutputManager` | 16 (Data Analytics) | Logging button events |
