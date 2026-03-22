## Build and Test

  ### Build the native program
  ```bash
  make

  This builds the default native target.

  ### Run the native test suite

  make test

  This builds and runs all native tests.

  ### Build the WebUI tests

  make web_test

  This builds the WebUI test artifacts and requires Emscripten (em++) to be installed and active in your environment.

  ### Build everything

  make all

  This builds the native program and runs the native tests.

  ### Clean build artifacts

  make clean
