> [!NOTE]
>
> This document elaborates Section 3.1 of the README.
> Section numbers here follow the global numbering (3.1.x).

# Group 18's Write-up for Initial C++ Class (WebUI)

*Last updated: Feb 27, 2026*

## 3.1 `WebImage` Class Specification

---

### 3.1.1 Class Description

A C++ class that represents an image used in the Web UI. It manages the image's source (URL or asset key), logical size, alternative text, opacity, and basic display properties such as visibility and error-handling behavior.

All code resides in `namespace cse498`, consistent with the project-wide convention used by `IDomElement`, `ICanvasElement`, and other WebUI classes.

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

/// Destructor removes the DOM element from the page and deregisters
/// from the internal event registry.
~WebImage();

// Copy operations are deleted (each instance owns a unique DOM element)
WebImage(const WebImage&) = delete;
WebImage& operator=(const WebImage&) = delete;

// Move operations transfer DOM ownership and update the event registry
WebImage(WebImage&& other) noexcept;
WebImage& operator=(WebImage&& other) noexcept;
```

#### Source & Metadata

```cpp
/// Set the image source (URL or asset path).
/// Resets the loaded/error state.
void SetSource(const std::string& src);
[[nodiscard]] std::string GetSource() const;

/// Set/get the alternative text for accessibility.
void SetAltText(const std::string& alt_text);
[[nodiscard]] std::string GetAltText() const;
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

[[nodiscard]] int GetWidth() const;
[[nodiscard]] int GetHeight() const;
```

> **Design note (per instructor feedback):** The old `SetMaintainAspectRatio(bool)` was
> ambiguous about when the ratio was enforced. `Resize()` makes it explicit:
> each call decides whether to preserve the ratio or stretch.

#### Opacity / Transparency

```cpp
/// Set the opacity (0.0 = fully transparent, 1.0 = fully opaque).
void SetOpacity(double alpha);
[[nodiscard]] double GetOpacity() const;
```

#### Visibility

```cpp
void Show();
void Hide();
[[nodiscard]] bool IsVisible() const;
```

#### Loading State & Error Handling

```cpp
[[nodiscard]] bool IsLoaded() const;
[[nodiscard]] bool HasError() const;

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

> **Note:** `MarkLoaded(bool)` is now a **private** method. External callers should
> not manually mark an image as loaded; loading state is managed internally by
> `HandleLoad()` (on successful load) and reset by `SetSource()`.

> **Design note (per instructor feedback):** When an image file is not found
> or fails to load, the default behavior (`BlankRect`) replaces the broken
> icon with a colored rectangle at the specified dimensions. Callers can
> switch to `NoOp` if they prefer to handle failures entirely in their
> own error callback.

#### IDomElement Interface

```cpp
void MountToLayout(WebLayout& parent, Alignment align) override;
void Unmount() override;
void SyncFromModel() override;
[[nodiscard]] const std::string& Id() const override;
```

> **Note:** The `MountToLayout` override does **not** repeat the base-class default
> argument (`Alignment::None`). Default arguments on virtual/override methods are
> prohibited by the project's Clang-Tidy rules to avoid subtle bugs where a
> different default in the derived class does not apply through a base-class pointer.

#### ICanvasElement Interface

```cpp
void Draw(WebCanvas& canvas) override;  // stub for future canvas rendering
```

---

### 3.1.4 Usage Example

```cpp
#include "WebImage.hpp"
#include <iostream>

using namespace cse498;

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

3. **Memory Management**: Each `WebImage` instance owns a DOM element. Proper cleanup is handled by the destructor (via the private `CleanupElement()` helper), but care must be taken when using move semantics. Move operations update the internal event registry so that JS callbacks continue to reach the correct C++ object.

4. **Canvas Rendering**: The `Draw()` method is currently a stub. Full canvas rendering would require additional implementation to draw the image onto a 2D canvas context.

---

### 3.1.7 Coordination with Other Classes/Groups

| Class | Interaction |
|-------|-------------|
| `WebLayout` | WebImage implements `IDomElement` and can be mounted to a layout via `MountToLayout()`. **Positioning is fully managed by WebLayout** (Flex/Grid/Free). WebImage does NOT control its own position. |
| `WebCanvas` | WebImage implements `ICanvasElement` and can be drawn on a canvas via `Draw()`. (Currently stub implementation) |
| `WebButton` / `WebTextbox` | Similar DOM manipulation patterns; shares the Emscripten `val` approach for JavaScript interop. |

---

### 3.1.8 File Structure

```
WebUI/WebImage/
├── WebImage.hpp      # Header with class declaration (namespace cse498)
├── WebImage.cpp      # Implementation using Emscripten
├── main.cpp          # Demo program
└── index.html        # Demo HTML page

tests/Interfaces/WebUI/
└── WebImageTest.cpp  # Catch2 unit tests (35 test cases, 110 assertions)
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

### 3.1.10 Unit Test Run Instructions

Tests are located at `tests/Interfaces/WebUI/WebImageTest.cpp` and use the Catch2 framework.

From the repository root, run all tests via the top-level Makefile:

```bash
make test
```

Or compile and run just the WebImage tests with Emscripten + Node.js:

```bash
em++ -std=c++23 -O2 -lembind -sALLOW_MEMORY_GROWTH \
  -sEXPORTED_FUNCTIONS='["_main","_WebImage_handleLoad","_WebImage_handleError"]' \
  tests/Interfaces/WebUI/WebImageTest.cpp \
  source/Interfaces/WebUI/WebImage/WebImage.cpp \
  source/Interfaces/WebUI/WebLayout/WebLayout.cpp \
  -I source -I third-party/Catch/single_include \
  --pre-js dom_shim.js \
  -o /tmp/WebImageTest.js && node /tmp/WebImageTest.js
```

Success is indicated by:

```text
===============================================================================
All tests passed (110 assertions in 35 test cases)
```

---

### 3.1.11 LLM Assistance Disclosure

The following parts of the WebImage module were produced or refined with the assistance of a large language model (LLM):

- **Unit tests (`WebImageTest.cpp`)**: The 35 Catch2 test cases were generated with LLM assistance. Each test was reviewed and validated to ensure correctness, full coverage of public member functions, and proper edge-case handling.
- **Code comments and documentation**: One-line `///` comments preceding every function implementation in `WebImage.cpp` were written with LLM assistance to address peer-review feedback requesting brief descriptions for each function.
- **Peer-review refactoring**: The LLM assisted in implementing changes requested during code review, including wrapping all code in `namespace cse498`, replacing unsafe `reinterpret_cast` with an integer-based event registry, extracting repeated DOM-removal logic into `RemoveFromDom()` / `CleanupElement()` helpers, adding `[[nodiscard]]` attributes, moving `MarkLoaded` to `private`, and replacing magic numbers with named constants.

All LLM-generated code was reviewed, tested, and integrated by the author.

---

### 3.1.12 Changelog

| Date | Changes |
|------|---------|
| Jan 30, 2026 | Initial specification |
| Feb 4, 2026 | First implementation with full API |
| Feb 12, 2026 | **v2**: Removed position control (handled by WebLayout); replaced `SetMaintainAspectRatio` with `Resize()`; added opacity support; added image error handling (`BlankRect` / `NoOp`); fixed include paths for new directory structure |
| Feb 27, 2026 | **v3 (peer-review)**: Wrapped all code in `namespace cse498`; removed default argument from `MountToLayout` override; added `[[nodiscard]]` to getters; moved `MarkLoaded` to private; extracted `RemoveFromDom()` / `CleanupElement()` helpers to eliminate repeated unmount code; replaced `reinterpret_cast<intptr_t>` with integer-keyed event registry; converted file-static helpers (`GetDocument`, `ToPx`) to private static methods; replaced hardcoded `"100px"` with `static constexpr int kDefaultPlaceholderPx`; added one-line `///` comments to all functions; updated tests to use `REQUIRE` for critical assertions and added category comments; added LLM assistance disclosure |
