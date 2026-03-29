# WebUI — Emscripten-based Web Interface (Group 18)

This subsystem provides C++ UI components (WebButton, WebCanvas, WebImage,
WebLayout, WebTextbox) that compile to WebAssembly via Emscripten and run in
a browser.

## Prerequisites

| Tool | Version | Notes |
|------|---------|-------|
| **Emscripten SDK** | latest | Provides `em++` compiler |
| **Python 3** | 3.x | For the local dev server |
| **Modern browser** | — | Chrome / Firefox / Edge |

## Setup

```bash
# 1. Clone and enter the Emscripten SDK (one-time)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# 2. Install & activate
./emsdk install latest
./emsdk activate latest

# 3. Source the environment (run every new shell session)
source ./emsdk_env.sh     # Linux / macOS
# emsdk_env.bat            # Windows
```

Verify the installation:

```bash
em++ --version
```

## Building WebUI Components' Demos

Individual components can be compiled with `em++`.

For WebButton:

```bash
cd source/Interfaces/WebUI/WebButton
em++ main.cpp WebButton.cpp ../WebLayout/WebLayout.cpp \
     -I../../../ --bind -std=c++23 \
     -s WASM=1 -s ASSERTIONS=1 \
     -o index.html
```

For WebCanvas, read [3.4.3](https://github.com/CSE498/Spring2026-CompanyA/blob/d134241584fe44b10db4e1d47f4efdaadfe8ef53/docs/ClassSpecs/Interfaces/WebUI/WebCanvas.md#343-stand-alone-demo--test) in `Spring2026-CompanyA/docs/ClassSpecs/Interfaces/WebUI/WebCanvas.md`

## Building Module Demo

The module demo main file is Group18_main.cpp, located in the source directory along with an index.html file. To build the demo, run
```bash
# From the repo root (Emscripten environment must be active)
cmake --build build --target group18_demo
```
then
```bash
# From the repo root (Emscripten environment must be active)
emrun /source/index.html --browser <chrome | edge | ...> --serve-root .
```
The --serve-root argument is important for loading image assets.

## Running Tests

WebUI tests are **excluded** from the default target because they
require the Emscripten toolchain. A dedicated cmake target compiles all
WebUI tests:

```bash
# From the repo root (Emscripten environment must be active)
cmake --build build --target web_test
```

This produces HTML files in `tests/build/web_tests/`. To run them:

```bash
cd tests/build/web_tests
python3 -m http.server 8000
```
or
```bash
emrun tests/build/web_tests --browser <chrome | edge | ...>
```

Then open `http://localhost:8000/<TestName>.html` in your browser. Test
results are shown on the page and in the browser console.

### Running a single test manually

```bash
cd tests/Interfaces/WebUI

em++ WebTextboxTest.cpp \
     ../../../source/Interfaces/WebUI/WebTextbox/WebTextbox.cpp \
     ../../../source/Interfaces/WebUI/WebLayout/WebLayout.cpp \
     -I../../../source -I../../../third-party/Catch/single_include \
     --bind -std=c++23 -s WASM=1 -s ASSERTIONS=1 \
     -DCATCH_CONFIG_MAIN \
     -o WebTextboxTest.html
```

## Project Structure

```
source/Interfaces/WebUI/
├── WebButton/       # Button component
├── WebCanvas/       # Canvas (immediate-mode drawing)
├── WebImage/        # Image component
├── WebLayout/       # Flex / Grid / Free layout manager
├── WebTextbox/      # Text display component
├── interface/       # Public interface docs
└── internal/        # Shared base classes (IDomElement, ICanvasElement)
```

## Notes

- All classes live in the `cse498` namespace.
- WebUI source files include `<emscripten/val.h>` and cannot be compiled
  with a standard (non-Emscripten) C++ compiler.
- The `source/Makefile` and `tests/Makefile` exclude `*/WebUI/*` paths so
  that `make build` and `make test` work for everyone without Emscripten.
