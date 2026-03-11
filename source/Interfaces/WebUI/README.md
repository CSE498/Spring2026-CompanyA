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

## Building WebUI Components

Individual components can be compiled with `em++`. Example for the
WebButton demo:

```bash
cd source/Interfaces/WebUI/WebButton
em++ main.cpp WebButton.cpp ../WebLayout/WebLayout.cpp \
     -I../../../ --bind -std=c++23 \
     -s WASM=1 -s ASSERTIONS=1 \
     -o index.html
```

## Running Tests

WebUI tests are **excluded** from the default `make test` target because they
require the Emscripten toolchain. A dedicated Makefile target compiles all
WebUI tests:

```bash
# From the repo root (Emscripten environment must be active)
make web_test
```

This produces HTML files in `tests/build/web_tests/`. To run them:

```bash
cd tests/build/web_tests
python3 -m http.server 8000
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
