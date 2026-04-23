# Group 15 — Generative World: Class Documentation

---
# How to run demo

---

**To run this demo:**
*    Step 1: follow the instructions in BUILD.md and CMAKE.md to build the executable
*    Step 2: cd into /demo, and then run ./Group15_demo
     **/

**the modules that we will be using.:**

#include "../source/Agents/PacingAgent.hpp"

#include "../source/Interfaces/TrashInterface.hpp"

#include "../source/Worlds/MazeWorld.hpp"

#include "../source/Worlds/DungeonOne.hpp"

---

# Dungeon Generation System


The dungeon generation works in layers: `BSP` splits the space into regions, `RoomHolder` manages room templates, `WorldGen` puts it all together, and `DungeonBase/DungeonOne` integrate it with the game world

### BSP (Binary Space Partitioning)
**Location:** 'source/core/BSP-Dungeon.hpp'

**What it is for:**
Splits a rectangular grid into smaller regions. Each node becomes a slot where a room gets placed then creates a varied dungeon layout without rooms overlapping.

### RoomHolder

**Location:** `source/core/RoomHolder.hpp`

**What it is for:**
Loads room templates from text files and picks which room to use

**Room File Format:**
```
^^^^^^^
<     >
<  m  >
<     >
&&&d&&&
```

**Character meanings:**

*Characters that are consistent across levels:*
| Char | What it is |
|------|------------|
| `f` | Top Secret door (use the same wall type that is on either side of the door as the wall) |
| `T` | Bottom Secret door (use the same wall type that is on either side of the door as the wall) |
| `U` | Left Secret door (use the same wall type that is on either side of the door as the wall) |
| `v` | Right Secret door (use the same wall type that is on either side of the door as the wall) |
| `s` | Monster spawn (skeleton) |
| `g` | Monster spawn (goblin) |
| `l` | Loot spawn (use floor alt 1 as the floor, layer a chest image on top)|
| `t` | Trap (use floor alt 1 for the level as the image, layer a trap agent on top of it)|

*Level 1 characters:*
| Char | What it is |
|------|------------|
| `a` | Floor (walkable) |
| `b` | Floor(alt 1) (walkable) |
| `c` | Floor(alt 2) (walkable) |
| `d` | Floor(alt 3) (walkable) |
| `<` | Floor(alt 4) (walkable) |
| `1` | Top wall |
| `2` | Bottom wall |
| `3` | Left wall |
| `4` | Right wall |
| `5` | Internal obstacle (version 1)|
| `6` | Internal obstacle (version 2)|
| `7` | Door (left verison) |
| `8` | Door (right verison) |
| `e` | Exit door (use the regular floor tile and layer a black circle on top) |

*Level 2 characters:*
| Char | What it is |
|------|------------|
| `A` | Floor (walkable) |
| `B` | Floor(alt 1) (walkable) |
| `C` | Floor(alt 2) (walkable) |
| `D` | Floor(alt 3) (walkable) |
| `E` | Floor(alt 4) (walkable) |
| `!` | Top wall |
| `@` | Bottom wall |
| `?` | Left wall |
| `$` | Right wall |
| `%` | Internal obstacle (version 1)|
| `^` | Internal obstacle (version 2)|
| `&` | Door (left verison) |
| `*` | Door (right verison) |
| `u` | Exit door (use the regular floor tile and layer a black circle on top) |

*Level 3 characters:*
| Char | What it is |
|------|------------|
| `m` | Floor (walkable) |
| `n` | Floor(alt 1) (walkable) |
| `o` | Floor(alt 2) (walkable) |
| `p` | Floor(alt 3) (walkable) |
| `q` | Floor(alt 4) (walkable) |
| `9` | Top wall |
| `0` | Bottom wall |
| `-` | Left wall |
| `=` | Right wall |
| `[` | Internal obstacle (version 1)|
| `]` | Internal obstacle (version 2)|
| `.` | Door (left verison) |
| `;` | Door (right verison) |
| `r` | Exit door (use the regular floor tile and layer a black circle on top) |

*Level 4 characters:*
| Char | What it is |
|------|------------|
| `M` | Floor (walkable) |
| `N` | Floor(alt 1) (walkable) |
| `O` | Floor(alt 2) (walkable) |
| `P` | Floor(alt 3) (walkable) |
| `Q` | Floor(alt 4) (walkable) |
| `(` | Top wall |
| `)` | Bottom wall |
| `_` | Left wall |
| `+` | Right wall |
| `{` | Internal obstacle (version 1)|
| `}` | Internal obstacle (version 2)|
| `~` | Door (left verison) |
| `:` | Door (right verison) |
| `R` | Exit door (use the regular floor tile and layer a black circle on top) |

### WorldGen
**Location:**
`source/core/WorldGeneration.hpp`

**What it is for:**
Orchestrates the full dungeon creation, runs BSP, places rooms, carves corridors between them


### DungeonBase
**Location:** `source/core/DungeonBase.hpp`

**What it's for:**  
Base class for dungeon worlds. Handles cell type registration, agent movement, and wall collision.

### DungeonOne
**Location:** `source/Worlds/DungeonOne.hpp`

**What it's for:**  
A concrete dungeon implementation with a specific room pool. Shows how to create dungeons.

---

## Items

### ItemBase
**Location:** `source/core/ItemBase.hpp`

**What it's for:**  
Base class for all non-agent items.


---


## Position System


### WorldPosition
**Location:** `source/core/WorldPosition.hpp`

This is the base class for all positions in the game. It stores x/y as doubles but also provides `CellX()` and `CellY()` to get the integer grid cell.

### WorldGridPosition
**Location:** `source/tools/WorldGridPosition.hpp`

**What it's for:**  
This extends `WorldPosition` for entities that need to track which direction they're facing and do grid-based checks (collision, adjacency, distance). Primarily intended for agents.

---

## Random Number Generation

### Random
**Location:** `source/tools/Random.hpp`

**What it's for:**  
Generating random numbers with reproducible sequences. Uses and adapted and customized version of Xoshiro256++ algorithm instead of `<random>`.

**How to use it**
After including Random.hpp, you get access to two main functions. GetValue() and P().
GetValue() is a templated function that allows you to randomly generate any type that you wish. It has two required parameter, min and max, to define the range of values you would like to generate a number between.
P() generated a weighted boolean value. It essentailly flips a weighted coin. The weight of the coin is set to 0.5 by default, but this may be changed with the probability parameter.
Additionally, if you have a specific seed you would like to use instead of the one created based on the time from your computer, you may set it with SetSeed().

---

### WeightedSet
**Location:** `source/tools/WeightedSet.hpp`

**What it's for:**  
A container where each item has a weight, and you can sample randomly based on those weights. Used for room selection in dungeon generation, but could be used for loot tables, spawn rates, etc.

---

## Grid System

The grid system has two layers: `WorldGrid` and `WorldGridState`.

### WorldGrid
**Location:** `source/core/WorldGrid.hpp`

**What it's for:**  
The fundamental 2D grid that holds the game world. Each cell has a type ID that maps to a `CellType`

### WorldGridState
**Location:** `source/tools/WorldGridState.hpp`

**What it's for:**  
Extends `WorldGrid` with cell modifiers — properties like whether a cell is traversable, breakable, or locked. This lets you distinguish between different wall types without hardcoding checks.
