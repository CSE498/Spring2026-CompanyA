> [!NOTE]
>
> This document elaborates Section 3.3 of the README.
> Section numbers here follow the global numbering (3.3.x).

# Group 18's Write-up for Initial C++ Class (WebUI)

*Last updated: Feb 16, 2026*

## 3.3 `WebButton` Class Specification

---

### 3.3.1 Class Description

A C++ class that represents a clickable button in the Web UI. It manages an HTML `<button>` element via Emscripten, allowing a programmer to control text, size, colors, enabled/disabled state, visibility, and a click callback from C++.

`WebButton` is a reusable UI component: it stores *what* the button looks like and *how* it behaves, but **not** *where* it is rendered. Positioning is handled by `WebLayout` (Flex/Grid/Free).

The class implements the **`IDomElement`** interface for integration with `WebLayout` (DOM-based rendering).

---

### 3.3.2 Similar Standard Library Classes

- **`std::function`** - for storing the click callback
- **`std::string`** - for storing label text
- **Emscripten's `val`** - for JavaScript/DOM interaction

---

### 3.3.3 Key Functions

#### Construction & Destruction

```cpp
explicit WebButton(const std::string& label = "");
~WebButton();

WebButton(const WebButton&) = delete;
WebButton& operator=(const WebButton&) = delete;

WebButton(WebButton&& other) noexcept;
WebButton& operator=(WebButton&& other) noexcept;
```

#### Label

```cpp
void SetLabel(const std::string& text);
std::string GetLabel() const;
```

#### Callback

```cpp
void SetCallback(std::function<void()> callback);
void Click();
```

#### Size

```cpp
void SetSize(int width, int height);
int GetWidth() const;
int GetHeight() const;
```

#### Styling

```cpp
void SetBackgroundColor(const std::string& color);
void SetTextColor(const std::string& color);
```

#### State

```cpp
void Enable();
void Disable();
bool IsEnabled() const;
void Show();
void Hide();
bool IsVisible() const;
```

#### IDomElement Interface

```cpp
void mountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;
void unmount() override;
void syncFromModel() override;
const std::string& Id() const override;
```

---

### 3.3.4 Error Conditions

| Type | Example | Handling |
|------|---------|----------|
| Programmer error | Negative width/height, null callback | `assert()` |
| User error | Clicking disabled button | Ignore silently |

Following instructor guidance, **no C++ exceptions** will be thrown inside `WebButton` for WebAssembly builds.

---

### 3.3.5 Expected Challenges

- Learning Emscripten's C++ to JavaScript bridging (embind, val)
- DOM manipulation from C++
- Memory cleanup (removing DOM elements, unbinding listeners)
- Coordinating with WebLayout for proper mount/unmount lifecycle

---

### 3.3.6 Coordination with Other Classes

| Class | Group | Reason |
|-------|-------|--------|
| `WebLayout` | 18 | Manages button positioning via IDomElement interface |
| `WebTextbox` | 18 | Shared IDomElement pattern, consistent styling approach |
| `WebImage` | 18 | Shared IDomElement pattern, consistent lifecycle |
| `WebCanvas` | 18 | Optional: visual representation of buttons inside canvas |
| `ActionMap` | 2 (Classic Agents) | Similar string-to-function mapping pattern |
| `Menu` | 17 (GUI Interface) | Common interface for both GUI systems |
| `OutputManager` | 16 (Data Analytics) | Logging button events |

---

### 3.3.7 File Structure

```
WebUI/WebButton/
  WebButton.hpp        # Header with class declaration
  WebButton.cpp        # Implementation using Emscripten
  WebButtonTest.cpp    # Unit tests (Catch2)
  main.cpp             # Demo program
  index.html           # Demo HTML page
```

---

### 3.3.8 Unit Test Run Instructions (`WebButtonTest.cpp`)

The test file uses Catch2 and can be compiled natively (without Emscripten) since it stubs out DOM dependencies. From the repo root:

```bash
make test
```

Or compile directly:

```bash
g++ -std=c++20 -I third-party/Catch/single_include \
  WebButtonTest.cpp -o webbutton_test && ./webbutton_test
```

---

### 3.3.9 Changelog

| Date | Changes |
|------|---------|
| Jan 30, 2026 | Initial specification |
| Feb 16, 2026 | **v2**: Removed position control (handled by WebLayout); implemented IDomElement interface (mountToLayout, unmount, syncFromModel, Id); added unique ID generation; added GetWidth/GetHeight; updated tests to Catch2; aligned patterns with WebImage and WebTextbox |