> [!NOTE]
>
> This document elaborates Section 3.1 of the README.
> Section numbers here follow the global numbering (3.1.x).

# Group 18's Write-up for Initial C++ Class (WebUI)

*Last updated: Feb 4, 2026*

## 3.1 `WebImage` Class Specification

---

### 3.1.1 Class Description

A C++ class that represents an image used in the Web UI. It manages the image's source (URL or asset key), logical size, position, alternative text, and basic display properties such as visibility and aspect-ratio behavior.

`WebImage` is designed as a reusable content container: it stores *what* image to show and *how* it should look (dimensions, alt text, layout hints), but not *where* it is rendered. Rendering can be performed either:

- as a DOM `<img>` element via `WebLayout`, or
- as a drawn sprite/texture inside an HTML5 `<canvas>` via `WebCanvas`.

The class implements two interfaces:
- **`IDomElement`** – for integration with `WebLayout` (DOM-based rendering)
- **`ICanvasElement`** – for integration with `WebCanvas` (canvas-based rendering)

The goal is to provide a simple C++ interface for using images in the game without requiring callers to manually write JavaScript or low-level rendering code.

---

### 3.1.2 Similar Standard Library Classes

- **`std::string`** – for storing image source paths, alt text, and CSS-like hints  
- **`std::optional`** – for representing optional layout hints or metadata  
- **`std::function`** – for load-completion callbacks or notification hooks  
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
/// Changing the source resets the loaded state.
void SetSource(const std::string& src);

/// Get the current image source.
std::string GetSource() const;

/// Set the alternative text for accessibility.
void SetAltText(const std::string& alt_text);

/// Get the alternative text.
std::string GetAltText() const;
```

#### Geometry (logical size & position, in pixels)

```cpp
/// Set the display size of the image.
/// @param width_px  Width in pixels (must be >= 0)
/// @param height_px Height in pixels (must be >= 0)
void SetSize(int width_px, int height_px);

/// Get the display width.
int GetWidth() const;

/// Get the display height.
int GetHeight() const;

/// Set the position of the image (absolute positioning).
void SetPosition(int x, int y);

/// Get the X coordinate.
int GetX() const;

/// Get the Y coordinate.
int GetY() const;
```

#### Aspect Ratio & Fitting

```cpp
/// Enable or disable maintaining aspect ratio when resizing.
/// When enabled, uses CSS `object-fit: contain`.
/// When disabled, uses CSS `object-fit: fill` (stretches to fit).
void SetMaintainAspectRatio(bool enabled);

/// Check if aspect ratio is being maintained.
bool GetMaintainAspectRatio() const;
```

#### Visibility

```cpp
/// Show the image (sets CSS display to default).
void Show();

/// Hide the image (sets CSS display to "none").
void Hide();

/// Check if the image is visible.
bool IsVisible() const;
```

#### Loading State

```cpp
/// Mark the image as loaded or not (for tracking async loading).
void MarkLoaded(bool loaded);

/// Check if the image has been loaded.
bool IsLoaded() const;

/// Set a callback to be invoked when the image finishes loading.
/// The callback is triggered by the browser's "load" event.
void SetOnLoadCallback(std::function<void()> callback);

/// Handle load event (called internally when image finishes loading).
void HandleLoad();
```

#### IDomElement Interface

```cpp
/// Mount this image to a WebLayout container.
/// @param parent The parent layout to mount to.
/// @param align  Alignment within the layout (default: Start).
void mountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;

/// Unmount this image from its parent layout.
void unmount() override;

/// Synchronize DOM element properties from the model state.
/// Re-applies all properties (src, alt, size, position, visibility).
void syncFromModel() override;

/// Get the unique DOM element ID.
const std::string& Id() const override;
```

#### ICanvasElement Interface

```cpp
/// Draw the image on a WebCanvas.
/// (Stub implementation - primarily supports DOM-based rendering)
void draw(WebCanvas& canvas) override;
```

---

### 3.1.4 Usage Example

```cpp
#include "WebImage.hpp"
#include <iostream>

int main() {
  // Create an image with URL and alt text
  WebImage img("https://example.com/image.png", "Example Image");
  
  // Set size and position
  img.SetSize(200, 150);
  img.SetPosition(50, 100);
  
  // Configure aspect ratio behavior
  img.SetMaintainAspectRatio(true);  // Preserve aspect ratio
  
  // Set up load callback
  img.SetOnLoadCallback([]() {
    std::cout << "Image loaded successfully!" << std::endl;
  });
  
  // Control visibility
  img.Show();   // Make visible
  img.Hide();   // Hide the image
  
  // Change source dynamically
  img.SetSource("https://example.com/another.png");
  
  // Check state
  if (img.IsLoaded() && img.IsVisible()) {
    std::cout << "Image is ready and visible" << std::endl;
  }
  
  return 0;
}
```

---

### 3.1.5 Error Conditions

| Condition | Behavior |
|-----------|----------|
| `SetSize()` with negative values | Assertion failure (debug mode) |
| `SetSource()` with invalid URL | Image shows broken icon; `IsLoaded()` remains false |
| Move from moved-from object | Moved-from object has null element; safe to destroy |
| DOM element removed externally | Methods check for null element before operations |

---

### 3.1.6 Expected Challenges

1. **Asynchronous Loading**: Images load asynchronously in the browser. The `onload` callback mechanism addresses this, but callers must handle the possibility that an image is not yet ready.

2. **Cross-Origin Resources**: Loading images from different domains may be subject to CORS restrictions. The implementation does not handle CORS headers automatically.

3. **Memory Management**: Each `WebImage` instance owns a DOM element. Proper cleanup is handled by the destructor, but care must be taken when using move semantics.

4. **Canvas Rendering**: The `draw()` method is currently a stub. Full canvas rendering would require additional implementation to draw the image onto a 2D canvas context.

---

### 3.1.7 Coordination with Other Classes/Groups

| Class | Interaction |
|-------|-------------|
| `WebLayout` | WebImage implements `IDomElement` and can be mounted to a layout via `mountToLayout()`. The layout manages positioning within its container. |
| `WebCanvas` | WebImage implements `ICanvasElement` and can be drawn on a canvas via `draw()`. (Currently stub implementation) |
| `WebButton` | Similar DOM manipulation patterns; shares the Emscripten `val` approach for JavaScript interop. |

---

### 3.1.8 File Structure

```
WebImage/
├── WebImage.hpp      # Header file with class declaration
├── WebImage.cpp      # Implementation using Emscripten
├── main.cpp          # Demo/test program
└── index.html        # Test HTML page
```

### 3.1.9 Build Instructions

Compile with Emscripten:

```bash
em++ -std=c++23 -O2 \
  -s WASM=1 \
  -s EXPORTED_FUNCTIONS="['_main', '_WebImage_handleLoad']" \
  -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" \
  --bind \
  -I.. \
  WebImage.cpp ../WebLayout/WebLayout.cpp main.cpp \
  -o webimage_demo.js
```

Run with a local server:

```bash
python3 -m http.server 8080
# Open http://localhost:8080/index.html
```
