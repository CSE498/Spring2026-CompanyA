> [!NOTE]
>
> This document elaborates Section 3.4 of the README.
> Section numbers here follow the global numbering (3.4.x).

# Group 18's Write-up for Initial C++ Class (WebUI)

*Last updated: Mar 21, 2026*

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

### 3.4.3 Stand Alone Demo & Test

**Run Demo with**

Powershell + Docker (under repo root):

```powershell
docker run --rm -it -v ${PWD}:/work -w /work emscripten/emsdk:latest bash -lc "em++ -std=c++23 -Wall -Wextra -Wpedantic -O2 \
source/Interfaces/WebUI/WebCanvas/WebCanvas.cpp \
source/Interfaces/WebUI/WebCanvas/WebCanvasDemo.cpp \
source/Interfaces/WebUI/WebImage/WebImage.cpp \
source/Interfaces/WebUI/WebButton/WebButton.cpp \
source/Interfaces/WebUI/WebTextbox/WebTextbox.cpp \
source/Interfaces/WebUI/WebLayout/WebLayout.cpp \
--js-library source/Interfaces/WebUI/WebCanvas/WebCanvas.js \
--bind \
-s WASM=1 \
-s ENVIRONMENT=web \
-s MODULARIZE=1 \
-s EXPORT_ES6=1 \
-s ALLOW_MEMORY_GROWTH=1 \
-s EXIT_RUNTIME=0 \
-o source/Interfaces/WebUI/WebCanvas/WebCanvasDemo.js"
```

Local (under repo root):

```powershell
em++ -std=c++23 -Wall -Wextra -Wpedantic -O2 \
  source/Interfaces/WebUI/WebCanvas/WebCanvas.cpp \
  source/Interfaces/WebUI/WebCanvas/WebCanvasDemo.cpp \
  source/Interfaces/WebUI/WebImage/WebImage.cpp \
  source/Interfaces/WebUI/WebButton/WebButton.cpp \
  source/Interfaces/WebUI/WebTextbox/WebTextbox.cpp \
  source/Interfaces/WebUI/WebLayout/WebLayout.cpp \
  --js-library source/Interfaces/WebUI/WebCanvas/WebCanvas.js \
  --bind \
  -s WASM=1 \
  -s ENVIRONMENT=web \
  -s MODULARIZE=1 \
  -s EXPORT_ES6=1 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s EXIT_RUNTIME=0 \
  -o source/Interfaces/WebUI/WebCanvas/WebCanvasDemo.js
```

**Run Tests with**

Powershell + Docker (under repo root):

```powershell
docker run --rm -it -v ${PWD}:/work -w /work emscripten/emsdk:latest bash -lc "em++ -std=c++23 -Wall -Wextra -Wpedantic -g -O0 -pthread \
-I third-party/Catch/single_include \
source/Interfaces/WebUI/WebCanvas/WebCanvas.cpp \
source/Interfaces/WebUI/WebImage/WebImage.cpp \
source/Interfaces/WebUI/WebButton/WebButton.cpp \
source/Interfaces/WebUI/WebTextbox/WebTextbox.cpp \
source/Interfaces/WebUI/WebLayout/WebLayout.cpp \
tests/Interfaces/WebUI/WebCanvasTest.cpp \
-D CATCH_CONFIG_MAIN \
--js-library source/Interfaces/WebUI/WebCanvas/WebCanvas.js \
--bind \
-o run_tests && node run_tests"
```

Local (under repo root):

```powershell
em++ -std=c++23 -Wall -Wextra -Wpedantic -g -O0 -pthread \
  -I third-party/Catch/single_include \
  source/Interfaces/WebUI/WebCanvas/WebCanvas.cpp \
  source/Interfaces/WebUI/WebImage/WebImage.cpp \
  source/Interfaces/WebUI/WebButton/WebButton.cpp \
  source/Interfaces/WebUI/WebTextbox/WebTextbox.cpp \
  source/Interfaces/WebUI/WebLayout/WebLayout.cpp \
  tests/Interfaces/WebUI/WebCanvasTest.cpp \
  -D CATCH_CONFIG_MAIN \
  --js-library source/Interfaces/WebUI/WebCanvas/WebCanvas.js \
  --bind \
  -o run_tests && node run_tests
```

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