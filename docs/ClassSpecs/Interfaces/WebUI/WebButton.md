> [!NOTE]
>
> This document elaborates Section 3.3 of the README.
> Section numbers here follow the global numbering (3.3.x).

# Group 18's Write-up for Initial C++ Class (WebUI)

*Last updated: Apr 20, 2026*

## 3.3 `WebButton` Class Specification

---

### 3.3.1 Class Description

A C++ class that represents a clickable button in the Web UI. It manages an HTML `<button>` element via Emscripten, allowing a programmer to control text, size, colors, enabled/disabled state, visibility, and a click callback from C++.

`WebButton` is a reusable UI component: it stores *what* the button looks like and *how* it behaves, but **not** *where* it is rendered. Positioning is handled by `WebLayout` (Flex/Grid/Free).

The class implements the **`IDomElement`** interface for integration with `WebLayout` (DOM-based rendering) and the **`ICanvasElement`** interface for optional drawing onto a `WebCanvas`.

All classes live under the `cse498` namespace.

---

### 3.3.2 Similar Standard Library Classes

- **`std::function`** - for storing the click callback
- **`std::string`** - for storing label text
- **Emscripten's `val`** - for JavaScript/DOM interaction
- **`std::invocable` (C++20 concept)** - used to constrain the template `SetCallback` overload

---

### 3.3.3 Key Functions

#### Construction and Destruction

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

template<typename F>
    requires std::invocable<F>
void SetCallback(F&& callable);

void Click();
```

The template overload accepts any callable type (lambda, functor, function pointer) without requiring an explicit `std::function` wrap.

#### Size

```cpp
void SetSize(int width, int height);
int GetWidth() const;
int GetHeight() const;
```

Passing 0 for width or height means use the browser default.

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
void MountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;
void SyncFromModel() override;
```

`MountToLayout` delegates to `WebLayout::AddElement`. `SyncFromModel` pushes the current model state (label, size, colors, enabled, visible) to the DOM element.

The `Unmount()` and `Id()` methods are inherited from the `IDomElement` base class.

#### ICanvasElement Interface

```cpp
void SetCanvasRect(float x, float y, float w = -1.0f, float h = -1.0f);
void Draw(WebCanvas& canvas) override;
```

`SetCanvasRect` sets the position and size for canvas rendering. `Draw` renders the button background rectangle and centered label text onto a `WebCanvas`.

#### Internal

```cpp
void HandleClick();
```

Called by the JavaScript click listener trampoline to forward DOM click events into C++.

---

### 3.3.4 Error Conditions

| Type | Example | Handling |
|------|---------|----------|
| Programmer error | Negative width/height, null callback | `assert()` |
| User error | Clicking disabled button | Ignore silently |

Following instructor guidance, **no C++ exceptions** are thrown inside `WebButton` for WebAssembly builds.

---

### 3.3.5 Expected Challenges

- Learning Emscripten's C++ to JavaScript bridging (embind, val)
- DOM manipulation from C++
- Memory cleanup (removing DOM elements, unbinding listeners)
- Coordinating with WebLayout for proper mount/unmount lifecycle
- Canvas rendering as an alternative to DOM-based display

---

### 3.3.6 Coordination with Other Classes

| Class | Group | Reason |
|-------|-------|--------|
| `WebLayout` | 18 | Manages button positioning via IDomElement interface |
| `WebTextbox` | 18 | Shared IDomElement pattern, consistent styling approach |
| `WebImage` | 18 | Shared IDomElement pattern, consistent lifecycle |
| `WebCanvas` | 18 | ICanvasElement rendering for canvas-based display |
| `WebInterface` | 18 | Main menu buttons, pause menu, settings menu |
| `Color` | 18 | Compile-time color validation for default hex values |
| `ActionMap` | 2 (Classic Agents) | Similar string-to-function mapping pattern |
| `Menu` | 17 (GUI Interface) | Common interface for both GUI systems |

---

### 3.3.7 File Structure

```
source/Interfaces/WebUI/WebButton/
    WebButton.hpp          # Header with class declaration
    WebButton.cpp          # Implementation using Emscripten
    main.cpp               # Standalone demo program

tests/Interfaces/WebUI/
    WebButtonTest.cpp      # Unit tests (Catch2, 47 test cases)

demos/WebUI/
    Group18_main.cpp       # Full integrated demo
    index.html             # Demo HTML page
```

---

### 3.3.8 Build and Test Instructions

#### Building the demo (from repo root)

```bash
cmake -S . -B build
cmake --build build --target group18_demo
emrun demos/WebUI/index.html --no_browser --serve_root .
```

Then open `http://localhost:6931/demos/WebUI/index.html` in a browser.

#### Building and running tests (from repo root)

```bash
cmake --build build --target web_test
cd tests/build/web_tests
python3 -m http.server 8000
```

Then open `http://localhost:8000/WebButtonTest.html` in a browser. Test results appear in the browser console (F12).

---

### 3.3.9 Changelog

| Date | Changes |
|------|---------|
| Jan 30, 2026 | Initial specification |
| Feb 16, 2026 | Removed position control (handled by WebLayout); implemented IDomElement interface; added unique ID generation; added GetWidth/GetHeight; updated tests to Catch2 |
| Apr 20, 2026 | Added ICanvasElement interface (SetCanvasRect, Draw); added template SetCallback overload with std::invocable constraint; added cse498 namespace; updated IDomElement methods to PascalCase (MountToLayout, SyncFromModel); added compile-time color validation via Color utility; expanded test suite to 47 test cases; updated file structure and build instructions to CMake; added coordination with WebInterface and Color |