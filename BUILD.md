# Build and Test

## Configure and Build

From the project root, run:

```bash
cmake -S . -B build
````

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

## Running the `simple` Executable

After building, go to `source` and run:

```bash
./simple
```

## Running Non-Web Tests

After building, go to `tests/build` and run:

```bash
./run_tests
```

## Running Web Tests

### Prerequisites

1. Install and activate the Emscripten SDK:

```bash
emsdk install latest
emsdk activate latest
```

2. Source the environment:

```bash
source emsdk_env.sh
```

### Serve the Web Test Files

After building, serve the generated HTML files and open them in a browser:

```bash
cd tests/build/web_tests
python3 -m http.server 8000
```

Then visit:

```text
http://localhost:8000/<TestName>.html
```
