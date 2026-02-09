> [!NOTE]
>
> This document elaborates Section 3.1 of the README.
> Section numbers here follow the global numbering (3.1.x).

# Group 18's Write-up for Initial C++ Class (WebUI)

*Last updated: Jan 30, 2026*

## 3.1 `WebImage` Class Specification

---

### 3.1.1 Class Description

A C++ class that represents an image used in the Web UI. It manages the image’s source (URL or asset key), logical size, position, alternative text, and basic display properties such as visibility and aspect-ratio behavior.

`WebImage` is designed as a reusable content container: it stores *what* image to show and *how* it should look (dimensions, alt text, layout hints), but not *where* it is rendered. Rendering can be performed either:

- as a DOM `<img>` element via `WebLayout`, or
- as a drawn sprite/texture inside an HTML5 `<canvas>` via `WebCanvas`.

The goal is to provide a simple C++ interface for using images in the game without requiring callers to manually write JavaScript or low-level rendering code.

---

### 3.1.2 Similar Standard Library Classes

- **`std::string`** – for storing image source paths, alt text, and CSS-like hints  
- **`std::optional`** – for representing optional layout hints or metadata  
- **`std::function`** – for load-completion callbacks or notification hooks  
- **Emscripten’s `val`** (conceptually) – for possible integration with JavaScript-based asset loaders or DOM APIs in the implementation

---

### 3.1.3 Key Functions

```cpp
// Construction
WebImage(const std::string& src,
         const std::string& alt_text = "");
~WebImage();

// Source & Metadata
void SetSource(const std::string& src);
std::string GetSource() const;

void SetAltText(const std::string& alt_text);
std::string GetAltText() const;

// Geometry (logical size & position, in pixels)
void SetSize(int width_px, int height_px);
int GetWidth() const;
int GetHeight() const;

void SetPosition(int x, int y);
int GetX() const;
int GetY() const;

// Aspect Ratio & Fitting
void SetMaintainAspectRatio(bool enabled);
bool GetMaintainAspectRatio() const;

// Visibility
void Show();
void Hide();
bool IsVisible() const;

// Loading State (for async or delayed loading, if used)
void MarkLoaded(bool loaded);
bool IsLoaded() const;

void SetOnLoadCallback(std::function<void()> callback);