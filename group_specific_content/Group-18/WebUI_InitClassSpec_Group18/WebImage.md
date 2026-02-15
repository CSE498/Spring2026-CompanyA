> [!NOTE]
>
> This document elaborates Section 3.1 of the README.
> Section numbers here follow the global numbering (3.1.x).

# Group 18's Write-up for Initial C++ Class (WebUI)

*Last updated: Feb 12, 2026*

## 3.1 `WebImage` Class Specification

---

### 3.1.1 Class Description

A C++ class that represents an image used in the Web UI. It manages the image's source (URL or asset key), logical size, alternative text, opacity, and basic display properties such as visibility and error-handling behavior.

`WebImage` is designed as a reusable content container: it stores *what* image to show and *how* it should look (dimensions, alt text, transparency), but **not** *where* it is rendered. Positioning is handled by the layout system:

- as a DOM `<img>` element positioned by `WebLayout` (Flex / Grid / Free), or
- as a drawn sprite/texture inside an HTML5 `<canvas>` via `WebCanvas`.

The class implements two interfaces:
- **`IDomElement`** – for integration with `WebLayout` (DOM-based rendering)
- **`ICanvasElement`** – for integration with `WebCanvas` (canvas-based rendering)

The goal is to provide a simple C++ interface for using images in the game without requiring callers to manually write JavaScript or low-level rendering code.

---

### 3.1.2 Similar Standard Library Classes

- **`std::string`** – for storing image source paths, alt text, and CSS-like hints  
- **`std::function`** – for load/error callbacks  
- **Emscripten's `val`** – for DOM manipulation and JavaScript interoperability

---

### 3.1.3 Key Functions

#### Construction & Destruction

```cpp
/// Construct a WebImage with a source URL/path and optional alt text.
/// The image element is automatically created and appended to the DOM.
explicit WebImage(const std::string& src,
                  const std::string& alt_text = "");

/// Destructor removes the DOM element from the page.
~WebImage();

// Copy operations are deleted (each instance owns a unique DOM element)
WebImage(const WebImage&) = delete;
WebImage& operator=(const WebImage&) = delete;

// Move operations transfer DOM ownership
WebImage(WebImage&& other) noexcept;
WebImage& operator=(WebImage&& other) noexcept;
```

#### Source & Metadata

```cpp
/// Set the image source (URL or asset path).
/// Resets the loaded/error state.
void SetSource(const std::string& src);
std::string GetSource() const;

/// Set/get the alternative text for accessibility.
void SetAltText(const std::string& alt_text);
std::string GetAltText() const;
```

#### Sizing

```cpp
/// Set the display size of the image.
/// Does NOT preserve aspect ratio (stretches to exact dimensions).
void SetSize(int width_px, int height_px);

/// Resize the image with optional aspect-ratio preservation.
/// When maintain_aspect_ratio = true, the image scales to fit within
/// the bounding box (CSS object-fit: contain).
/// When false, stretches to exact dimensions (CSS object-fit: fill).
void Resize(int width_px, int height_px, bool maintain_aspect_ratio = false);

int GetWidth() const;
int GetHeight() const;
```

> **Design note (per instructor feedback):** The old `SetMaintainAspectRatio(bool)` was
> ambiguous about when the ratio was enforced. `Resize()` makes it explicit:
> each call decides whether to preserve the ratio or stretch.

#### Opacity / Transparency

```cpp
/// Set the opacity (0.0 = fully transparent, 1.0 = fully opaque).
void SetOpacity(double alpha);
double GetOpacity() const;
```

#### Visibility

```cpp
void Show();
void Hide();
bool IsVisible() const;
```

#### Loading State & Error Handling

```cpp
void MarkLoaded(bool loaded);
bool IsLoaded() const;
bool HasError() const;

/// Callback invoked when the image finishes loading.
void SetOnLoadCallback(std::function<void()> callback);

/// Callback invoked when the image fails to load.
void SetOnErrorCallback(std::function<void()> callback);

/// Set the behavior when an image source fails to load.
///   BlankRect – renders a colored placeholder rectangle (default).
///   NoOp     – does nothing; the element stays as-is.
void SetErrorMode(ImageErrorMode mode);

/// Set the placeholder color for BlankRect mode (any valid CSS color).
void SetPlaceholderColor(const std::string& css_color);
```

> **Design note (per instructor feedback):** When an image file is not found
> or fails to load, the default behavior (`BlankRect`) replaces the broken
> icon with a colored rectangle at the specified dimensions. Callers can
> switch to `NoOp` if they prefer to handle failures entirely in their
> own error callback.

#### IDomElement Interface

```cpp
void mountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;
void unmount() override;
void syncFromModel() override;
const std::string& Id() const override;
```

#### ICanvasElement Interface

```cpp
void draw(WebCanvas& canvas) override;  // stub for future canvas rendering
```

---

### 3.1.4 Usage Example

```cpp
#include "WebImage.hpp"
#include <iostream>

int main() {
  // Create an image
  WebImage img("assets/hero.png", "Hero sprite");

  // Resize keeping aspect ratio
  img.Resize(200, 150, true);

  // Set transparency
  img.SetOpacity(0.8);

  // Handle load success
  img.SetOnLoadCallback([]() {
    std::cout << "Image loaded!" << std::endl;
  });

  // Handle load failure
  img.SetErrorMode(ImageErrorMode::BlankRect);
  img.SetPlaceholderColor("#FF0000");
  img.SetOnErrorCallback([]() {
    std::cout << "Image failed to load." << std::endl;
  });

  // Check state
  if (img.HasError()) {
    std::cout << "Image has an error" << std::endl;
  }

  return 0;
}
```

---

### 3.1.5 Error Conditions

| Condition | Behavior |
|-----------|----------|
| `SetSize()` / `Resize()` with negative values | Assertion failure (debug mode) |
| `SetOpacity()` outside [0.0, 1.0] | Assertion failure (debug mode) |
| Image source not found / fails to load | `HasError()` returns true; behavior depends on `ImageErrorMode` |
| `SetSource()` with new URL | Resets `IsLoaded()` and `HasError()` state |
| Move from moved-from object | Moved-from object has null element; safe to destroy |
| DOM element removed externally | Methods check for null element before operations |

---

### 3.1.6 Expected Challenges

1. **Asynchronous Loading**: Images load asynchronously in the browser. The `onload`/`onerror` callback mechanism addresses this, but callers must handle the possibility that an image is not yet ready.

2. **Cross-Origin Resources**: Loading images from different domains may be subject to CORS restrictions. The implementation does not handle CORS headers automatically.

3. **Memory Management**: Each `WebImage` instance owns a DOM element. Proper cleanup is handled by the destructor, but care must be taken when using move semantics.

4. **Canvas Rendering**: The `draw()` method is currently a stub. Full canvas rendering would require additional implementation to draw the image onto a 2D canvas context.

---

### 3.1.7 Coordination with Other Classes/Groups

| Class | Interaction |
|-------|-------------|
| `WebLayout` | WebImage implements `IDomElement` and can be mounted to a layout via `mountToLayout()`. **Positioning is fully managed by WebLayout** (Flex/Grid/Free). WebImage does NOT control its own position. |
| `WebCanvas` | WebImage implements `ICanvasElement` and can be drawn on a canvas via `draw()`. (Currently stub implementation) |
| `WebButton` / `WebTextbox` | Similar DOM manipulation patterns; shares the Emscripten `val` approach for JavaScript interop. |

---

### 3.1.8 File Structure

```
WebUI/WebImage/
├── WebImage.hpp      # Header with class declaration
├── WebImage.cpp      # Implementation using Emscripten
├── test_WebImage.cpp # Unit tests
├── test_WebImage.html # Generated test page after build
├── main.cpp          # Demo/test program
└── index.html        # Test HTML page
```

### 3.1.9 Build Instructions

Compile with Emscripten (C++23):

```bash
em++ -std=c++23 -O2 \
  -s WASM=1 \
  -s EXPORTED_FUNCTIONS="['_main', '_WebImage_handleLoad', '_WebImage_handleError']" \
  -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" \
  --bind \
  -I.. -I../internal \
  WebImage.cpp ../WebLayout/WebLayout.cpp main.cpp \
  -o webimage_demo.js
```

Run with a local server:

```bash
python3 -m http.server 8080
# Open http://localhost:8080/index.html
```

### 3.1.10 Unit Test Run Instructions (`test_WebImage.cpp`)

From `WebUI/WebImage/`, compile tests with dependencies:

```bash
em++ test_WebImage.cpp WebImage.cpp ../WebLayout/WebLayout.cpp \
  -I../internal \
  -o test_WebImage.html \
  -s WASM=1 \
  --bind
```

Start a local server in the same directory:

```bash
python3 -m http.server 8000
# Open http://localhost:8000/test_WebImage.html
```

Open browser DevTools Console to read test results. Success is indicated by:

```text
Results: 35 passed, 0 failed, 35 total
All tests passed!
```

---

### 3.1.11 Changelog

| Date | Changes |
|------|---------|
| Jan 30, 2026 | Initial specification |
| Feb 4, 2026 | First implementation with full API |
| Feb 12, 2026 | **v2**: Removed position control (handled by WebLayout); replaced `SetMaintainAspectRatio` with `Resize()`; added opacity support; added image error handling (`BlankRect` / `NoOp`); fixed include paths for new directory structure |
