# Group 15 — Generative World: Class Documentation
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

| Char | What it is |
|------|------------|
| ` ` | Floor (walkable) |
| `^` | Top wall |
| `&` | Bottom wall |
| `<` | Left wall |
| `>` | Right wall |
| `$` | Internal obstacle |
| `d` | Door |
| `s` | Secret door |
| `m` | Monster spawn |
| `l` | Loot spawn |
| `t` | Trap |

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
Generating random numbers with reproducible sequences. Uses Xoshiro256++ algorithm instead of `<random>`.

---

### WeightedSet
**Location:** `source/tools/WeightedSet.hpp`

**What it's for:**  
A container where each item has a weight, and you can sample randomly based on those weights. Used for room selection in dungeon generation, but could be used for loot tables, spawn rates, etc.

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
