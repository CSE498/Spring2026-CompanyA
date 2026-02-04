<!-- Copilot / AI agent instructions for this repository -->
# Repository guidance for AI coding agents

This file contains concise, actionable guidance for an AI coding agent to be productive in this C++ course project.

- **Big picture**: The codebase is a small simulation framework. Core abstractions live in `source/core/`:
  - `Entity.hpp` — base for any object in the world (non-copyable, moveable).
  - `AgentBase.hpp` — agent-specific API: `SelectAction()`, `AddAction()`, `Notify()`, `GetActionResult()`.
  - `WorldBase.hpp` — world lifecycle and orchestration: `AddAgent<T>()`, `Run()`, `RunAgents()`, `DoAction()` (must be implemented by derived worlds).
  Example: `source/Worlds/MazeWorld.hpp` registers movement actions in `ConfigAgent()` and implements `DoAction()` to move agents.

- **Key directories / files** (read these first):
  - `source/core/` — main framework abstractions.
  - `source/Worlds/` — concrete world implementations (game rules, `DoAction`).
  - `source/Agents/` — agent implementations (implement `SelectAction`).
  - `source/Interfaces/` — human interface wrappers (derive from `AgentBase`/`InterfaceBase`).
  - `source/simple_main.cpp` — example program showing typical setup and `world.Run()` usage.
  - `source/Makefile` — build rules and targets (see Build / Run below).
  - `tests/` — test examples; project uses Catch (under `third-party/Catch/`).

- **Architecture patterns to follow** (discoverable by reading `core/*`):
  - Worlds own the `main_grid`, `agent_set`, and `item_set` and are responsible for agent lifecycle.
  - Agents are stored via `unique_ptr` and are non-copyable objects (IDs are their index in the vector).
  - Worlds provide actions via `ConfigAgent()` and implement action application via `DoAction()`.
  - Agents expose `SelectAction(const WorldGrid&)` and rely on the world API to query known agents/items.

- **Build / test / run workflows** (concrete commands):
  - Build default executable (named `simple`):
    ```bash
    make
    ```
  - Build debug binary with symbols:
    ```bash
    make debug-simple
    ```
  - Build optimized binary:
    ```bash
    make opt-simple
    ```
  - Clean generated files:
    ```bash
    make clean
    ```
  - Run example program (after `make`):
    ```bash
    ./simple
    ```
  - Tests: inspect `tests/` for usage of Catch; run test binaries produced by their own targets if present.

- **Repository-specific conventions**:
  - Compiler flags in `source/Makefile` target `c++23`. Keep new code compatible with C++23 features.
  - Header discovery: `Makefile` uses `find` to include `*.hpp` under `Agents core Interfaces tools Worlds`. If you add new headers, place them there or update the `Makefile`.
  - Entities are intentionally non-copyable — prefer `unique_ptr` ownership and `AddAgent<T>()` templated factory.
  - IDs are stable indices: `GetID()` correlates with the agent/item position in the owning vector.

- **Common code edits patterns** (examples to mirror):
  - To add a new world: derive from `WorldBase`, implement `DoAction()` and override `ConfigAgent()` to register actions. See `source/Worlds/MazeWorld.hpp`.
  - To add a new agent: implement `SelectAction()` in `source/Agents/` and use `world.AddAgent<MyAgent>()` in `simple_main.cpp`.
  - To add new cell types or load maps: use `WorldGrid` methods from world constructors (see `MazeWorld` constructor example).

- **Integration points & external dependencies**:
  - Tests rely on Catch located under `third-party/Catch/` — there is no package manager; tests are source-distributed here.
  - No external build systems (CMake) are required for the simple Makefile flow; some third-party folders contain CMake for the Catch project only.

- **When making changes, be careful of**:
  - Changing `Entity` copy semantics — many components rely on uniqueness and `unique_ptr` ownership.
  - Action ID conventions: 0 == no-action / remain still. Worlds typically define enums where 0 is no-op.
  - Makefile `EXECUTABLES` naming: executables map to `<name>_main.cpp` and produce a binary named `<name>`.

- **Next steps**: If you want, I can add example code snippets (small PRs) that show adding a new `Agent` or `World`.

If any part of this guide is unclear or you want more detail (examples, common one-line refactors, or test run commands), tell me which section to expand.
