# Build and Test

## Configure and Build

From the project root, run:

```bash
cmake -S . -B build
```

This generates a new `build` folder.

Then build the project with either:

```bash
cd build
make
```

or, from the project root:

```bash
cmake --build build
```

---

## Running the `simple` Executable

After building, go to `source` and run:

```bash
./simple
```

---

## Running Non-Web Tests

After building, go to `tests/build` and run:

```bash
./run_tests
```

---

## Running Web Tests

Web tests require the **Emscripten toolchain**. Complete the one-time setup below before building.

### Setup

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

### Build

WebUI tests are **excluded** from the default target because they require the Emscripten toolchain. A dedicated CMake target compiles all WebUI tests:

```bash
cmake --build build --target web_test
```

### Serve the Web Test Files

After building, serve the generated HTML files and open them in a browser. The web test files are output to `tests/build/web_tests`.

```bash
cd tests/build/web_tests
python3 -m http.server 8000
```

or

```bash
emrun tests/build/web_tests --browser <chrome | edge | ...>
```

Then open any of the following in your browser:

| Test | URL |
|---|---|
| Textbox | `http://localhost:8000/WebTextboxTest.html` |
| Image | `http://localhost:8000/WebImageTest.html` |
| Button | `http://localhost:8000/WebButtonTest.html` |
| Canvas | `http://localhost:8000/WebCanvasTest.html` |
| Layout | `http://localhost:8000/WebLayoutTest.html` |
| WebInterface | `http://localhost:8000/WebInterfaceTest.html` |

---

## Group Demos

Each group has a dedicated CMake target, **excluded** from the default build. Build a specific group's demo from the project root:

```bash
cmake --build build --target groupXX_demo
```

Replace `XX` with the zero-padded group number, e.g. `group07_demo`, `group12_demo`.

### Non-Web Demos
After building, run from the project root:

```bash
./demos/groupXX_demo
```

### Group 17 Demo (SDL)

Group 17's demo requires SDL. Install it by running:

```bash
make install
```

Two demo targets are available:

```bash
cmake --build build --target group17_demo   # standard demo
cmake --build build --target group17_demo2  # extended step-by-step walkthrough
```

### Group 18 Demo (Emscripten)

Group 18's demo requires the Emscripten toolchain. See the [Web Tests setup](#setup) section above if you haven't installed it yet.


> **Note:** Images use absolute paths from the project root, so `--serve-root .` is required for all `emrun` commands.

Open automatically in Chrome:

```bash
emrun demos/WebUI/index.html --browser chrome --serve_root .
```

Or start the server without opening a browser:

```bash
emrun demos/WebUI/index.html --no_browser --serve_root .
```

Then navigate to:
```
http://localhost:6931/demos/WebUI/index.html
```

Alternatively, run:
```bash
cd demos/WebUI
python3 -m http.server 8000
```