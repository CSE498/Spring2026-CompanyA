> [!NOTE]
>
> This document elaborates Section 3.4 of the README.
> Section numbers here follow the global numbering (3.4.x).

# Group 18's Write-up for Initial C++ Class (WebUI)

*Last updated: Jan 30, 2026*

## 3.4 `WebCanvas` Class Specification

---

### 3.4.1 Class Description

`WebCanvas` is the core rendering element of the Web Interface module. It manages an HTML5 `<canvas>` element and provides a C++ API to draw dynamic game content on the screen, ideally backed by a hardware-accelerated rendering pipeline (e.g., WebGL or an OpenGL-like context compiled via Emscripten).

Conceptually, `WebCanvas` plays a role similar to a video player: it maintains a continuous visual feed of the game world and UI overlay, driven entirely by C++ logic. It is responsible for:

- Initializing and managing the underlying `<canvas>` element and its rendering context.
- Managing canvas size, resolution, and coordinate systems (e.g., world-space vs. screen-space).
- Providing drawing operations for:
  - World tiles, backgrounds, and sprites.
  - Agents and interactive items.
  - UI overlays such as `WebImage`, `WebButton`, and `WebTextbox` when rendered within the canvas.
- Handling per-frame drawing (`BeginFrame` / `EndFrame`) and efficient clearing/updating.
- Offering hooks for higher-level modules (e.g., Agent API, World modules) to render their state.

`WebCanvas` does **not** decide layout for the rest of the page; that responsibility belongs to `WebLayout`. Instead, `WebCanvas` focuses solely on rendering into its own drawing surface, while `WebLayout` determines where that surface appears in the DOM and how it is combined with other web elements.

---

### 3.4.2 Similar Standard Library Classes

While there is no direct standard library equivalent to an HTML5 canvas, the following classes and concepts are relevant:

- **`std::vector`** – for storing vertex lists, sprite batches, or collections of drawable objects.
- **`std::pair` / `std::tuple`** – for representing coordinates, dimensions, and color components.
- **`std::chrono`** – for timing and frame-rate control (e.g., delta time between frames).
- **`std::function`** – for callbacks tied to frame updates or custom draw routines.
- **Emscripten’s `val`** (conceptually) – for bridging to JavaScript APIs such as the Canvas 2D context or WebGL context.

---

### 3.4.3 Key Functions

```cpp
// Construction & Initialization
WebCanvas(const std::string& canvasId);  // logical ID or DOM id for the canvas
~WebCanvas();

// Canvas Size & Coordinate System
void SetSize(int widthPx, int heightPx);   // logical pixel size
int GetWidth() const;
int GetHeight() const;

void SetWorldBounds(float worldWidth, float worldHeight); // for world-to-screen mapping
void SetCameraCenter(float worldX, float worldY);         // camera position in world space;
                                                          // WebCanvas will map to screen

// Frame Control
void BeginFrame();   // prepare canvas for a new frame (clear buffers, reset transforms)
void EndFrame();     // flush any pending draw calls; present the frame

// Basic Drawing Primitives
void Clear(const std::string& cssColor = "#000000");  // clear with color
void DrawRect(float x, float y, float width, float height,
              const std::string& fillColor);
void DrawLine(float x1, float y1, float x2, float y2,
              float lineWidth,
              const std::string& strokeColor);
void DrawCircle(float centerX, float centerY, float radius,
                const std::string& fillColor);

// Drawing Web Interface Items
void DrawImage(const WebImage& image,
               float x, float y,
               float width, float height);

void DrawTextbox(const WebTextbox& textbox,
                 float x, float y,
                 float maxWidth = -1.0f); // -1 => use intrinsic width

void DrawButton(const WebButton& button,
                float x, float y,
                bool isPressed = false);

// Advanced / Optional Features
void SetBackgroundColor(const std::string& cssColor);
std::string GetBackgroundColor() const;

void EnableSmoothing(bool enabled);   // turn anti-aliasing on/off if supported
bool IsSmoothingEnabled() const;

void SetOnFrameRendered(std::function<void()> callback);  // optional post-frame hook

// Status & Diagnostics
bool IsInitialized() const;   // true if canvas & context are ready
bool HasError() const;        // indicates a persistent canvas/context issue
std::string GetLastError() const;
```

The exact implementation (e.g., Canvas 2D API vs. WebGL, use of batching, etc.) is left open for later design, but the above interface is intended to provide a clean, C++-centric abstraction of an HTML5 canvas that integrates tightly with the rest of the Web Interface module.

------

### 3.4.4 Error Conditions

| Type              | Example                                                      | Handling                                                     |
| ----------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| Programmer error  | Negative width/height; invalid world bounds; calling `Draw*` before initialization | `assert()`                                                   |
| Recoverable error | Canvas or rendering context fails to initialize; lost WebGL context | Return `false` from initialization helpers and set internal error flags; log to web console |
| User error        | Attempting to draw an object that has been marked invisible or not fully loaded | Safely ignore the draw request or draw a placeholder; optionally record a non-fatal warning flag |

Following instructor guidance and our README’s performance considerations, **no C++ exceptions will be thrown inside `WebCanvas` for WebAssembly builds**.

- Programmer mistakes are handled via `assert()` (which can be compiled out in release builds).
- Recoverable errors are signaled through boolean return values, status flags (`HasError()`), and debug logging to the web console or a shared error manager.
- If needed, native (non-Emscripten) builds may later introduce optional exception-based paths, but this is outside the initial scope.

------

### 3.4.5 Expected Challenges

- **Emscripten + Canvas/WebGL integration**
  - Learning how to obtain the canvas element and rendering context from C++ via Emscripten.
  - Choosing between the Canvas 2D API and a WebGL (OpenGL-like) approach and adapting accordingly.
- **Performance & batching**
  - Avoiding excessive C++ ↔ JavaScript calls per frame.
  - Designing draw APIs that allow batching (e.g., drawing many sprites in one call or using buffers).
- **Coordinate systems & camera control**
  - Mapping world coordinates (from the World/Agent modules) to canvas pixels.
  - Handling zoom, pan, and possibly rotation, especially for “god mode” views.
- **Resolution & scaling**
  - Supporting different window sizes and pixel densities (including high-DPI screens).
  - Keeping aspect ratios consistent across different layouts and devices.
- **Debugging cross-language issues without exceptions**
  - Tracking down rendering errors that may originate in either C++ or JavaScript.
  - Building simple diagnostics (e.g., debug overlays or logging) directly into `WebCanvas`.

------

### 3.4.6 Coordination with Other Classes

| Class / Module                      | Group                              | Reason                                                       |
| ----------------------------------- | ---------------------------------- | ------------------------------------------------------------ |
| **`WebLayout`**                     | 18 (Web Interface)                 | `WebLayout` manages where the canvas appears in the DOM and how it interacts with other HTML elements; `WebCanvas` focuses on drawing into that surface. |
| **`WebImage`**                      | 18                                 | `WebCanvas` uses `WebImage` as a logical description of image content when drawing sprites or textures inside the canvas. |
| **`WebTextbox`**                    | 18                                 | Used to render text overlays (HUD, labels, debug info) directly into the canvas when needed. |
| **`WebButton`**                     | 18                                 | Optional: visual representation of buttons inside the canvas (e.g., in full-screen or custom UI modes). |
| **GUI Interface (Text/Image/Menu)** | 17 (GUI Interface)                 | Coordinate a shared “GUI Interface” so that both local GUI and web UI versions expose similar drawing concepts and can render the same World/Agent state. |
| **Agent API**                       | 1 & 2 (AI Agents / State Managers) | `WebCanvas` needs to query agent/world state via the Agent API to render positions, animations, and relevant game information. |
| **World modules**                   | 14 & 15 (World / World Utilities)  | Provide map layouts, tile data, and other world geometry to be visualized on the canvas. |
| **Output/Data Analytics**           | 16 (Data Analytics)                | Optionally render analysis overlays (heatmaps, paths, etc.) on top of the base game view for debugging or post-run review. |