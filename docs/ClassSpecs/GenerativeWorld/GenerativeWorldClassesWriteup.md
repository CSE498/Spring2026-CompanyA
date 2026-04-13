# Group 15 — Generative World: Class Specifications

---

## Random

**Lead:** Rachel Loren

### Description
Through the implementation of various functions, this class allows the user to generate a random number. The user can input a range of numbers to generate between, and a seed to use as the generator. If the user does not supply a range, the class will use a default range that depends on what specific function they are calling. If the user does not supply a seed, the program will use the current time off of the computer.

### Similar Classes
The standard library for C++ has a variety of pre-existing functions and classes to handle random number generation. The `<random>` header has a wide variety of different functions to randomly generate all sorts of things. The most common way to do this is to create a `std::random_device` (to set the seed), a `std::mt19937` (the generator), and a `std::uniform_int_distribution` (to set a range).

### Key Functions
- `int GetInt(int i_min = 0, int i_max = 100)`
- `double GetDouble(double d_min = 0.0, double d_max = 100.0)`
- `float GetFloat(float f_min = 0.00f, float f_max = 100.00f)`
- `char GetChar(char c_min = 'A', char c_max = 'Z')`
- `bool GetBool(bool b_min = false, bool b_max = true)`
- `bool P(double probability = 0.5)` — randomly returns true/false weighted by the given probability
- `void SetSeed(long long seed)` — allows the user to manually set and change the seed; defaults to current time on construction

Additional functions can be added to handle other data types if other groups have a use for it.

### Error Conditions
- Each `Get[Type]` function checks that min and max are supplied in the correct order. Invalid input is considered a programmer error and will throw an exception.
- Each `Get[Type]` function may throw an exception if there is insufficient memory to generate a value.

### Expected Challenges
No major challenges are anticipated. There may be some difficulty in applying something like the xoshiro shift algorithm across all the different data types.

### Dependencies on Other Groups
The Random class does not depend on any other group's classes. However, the following classes may want to use it: TagManager (Group 1), BehaviorTree (Group 2), PathGenerator (Group 2), Scheduler (Group 14), DataLog (Group 16), ActionLog (Group 16).

---

## WeightedSet

**Lead:** Abigail Franzmeier

### Description
This class provides a data structure for storing a collection of items that each have a numerical weight. The user can add and delete weighted items. The collection tracks total weight and allows indexing into the set by searching for any number within an item's weight range. An item's range begins at the sum of all weights before it and ends at that sum plus its own weight.

### Similar Classes
Similar classes include `std::set`, `std::multiset`, `std::map`, and `std::multimap`. These use an underlying red-black tree, which is the planned data structure for this class as well.

### Key Functions
- `Insert(item, weight)`
- `Remove(item)`
- `UpdateWeight(item, new_weight)`
- `FindItem(weight)`
- `GetTotalWeight()`
- `Size()` — returns the number of elements
- `IsEmpty()`

Internal rebalancing functions will also be needed since the underlying data structure is a red-black tree.

### Error Conditions
- **Programmer errors:** Assertions can be used to check that red-black tree properties hold after rebalancing.
- **Potentially recoverable:** Memory limitations may cause errors when adding new nodes.
- **User errors:** Invalid weights (weight < 0), indexing by weight out of range, or updating/deleting non-existent items.

### Expected Challenges
The main challenge will be performance, as the set may be large and needs to return data quickly and accurately.

### Dependencies on Other Groups
This class likely won't depend on another group's class. Within the group, `Random` will be used to index into the set in order to generate items, rooms, and enemies.

---

## DataFileManager

**Lead:** Zhixiang Miao

### Description
Manages files and data — processing files, information, and data while performing sanity checks and detecting bugs. Manages input/output files. Designs and collects data such as values, abilities, difficulty levels, player level requirements for specific areas, bosses, and weapon type and rarity.

### Similar Classes
- `<fstream>` — for file input and output
- `<vector>` — to store dungeon levels, dungeon difficulty, weapon types, weapon rarities, enemy types, enemy levels, enemy abilities, boss types, and boss difficulty
- `<map>` — to connect specific dungeons to their enemies, and chest rewards to dungeon difficulty

### Key Functions
- `OpenFile()`
- `ExitFile()`
- `InformationOfCharacter()` — returns specific information about the character
- `InformationOfEnemy()` — returns specific information about enemies; supports designing various enemy and boss types
- `InformationOfWeapon()` — designs weapon types, attributes, and rarity
- `InformationOfDungeon()` — handles dungeon level relative to player level and dungeon difficulty
- `InformationOfChest()` — determines chest rarity by color/style: Normal (white), Rare (yellow), Refined (blue), Epic (purple), Legendary (orange), corresponding to the rarity of the reward inside
- `Attributes()` — defines categories of abilities in the game world, each with a specific value; player attribute values can increase through leveling up or equipping items

### Error Conditions
- Risk of distributing weapons at incorrect levels (too high or too low).
- Risk of placing players in incorrect dungeons (too high or too low relative to their level).
- Risk of incorrect chest reward distribution — for example, a nightmare difficulty dungeon should yield epic or legendary items, not normal ones.

### Expected Challenges
Large amounts of data to process, as all values (attributes, levels, powers of both player and enemies) need to be designed.

### Dependencies on Other Groups
This class won't require other classes to function, but will need collaboration on data formats. Potential collaborators: TagManager, FeatureVector, MemoryFactory (Group 1); WorldPath, PathGenerator, MemoFunction (Group 2); DataMap, EventQueue (Group 14); DataLog, ActionLog (Group 16).

---

## StateGrid

**Lead:** Paul Bui

### Description
A dynamic 2D grid where each position is one of a set of pre-defined states. Each state type has a name, a symbol, and any other information that defines that state (perhaps as a DataMap). For example, a StateGrid representing a maze might define a "wall" state with symbol `#` and a "Hardness" property of 20, and an "open" state with symbol ` ` and no properties.

### Similar Classes
- `<vector>` — for dynamically changing storage to hold cell types
- `<cassert>` / GTest — for unit tests and debugging coordinates and tiles
- `<memory>` — for smart pointers to dynamically create/delete tiles when moving between rooms

### Key Functions
- `GetCellInfo()` — returns information about the type, properties, and interactability of a cell; useful for debugging and supporting tile types like spike traps, environmental obstacles, and interactable items
- `IsActive()` — returns whether the tile map is currently active; a false state means the map will be redrawn
- `TileInfo` (struct) — holds tile properties and associated values such as `wallState`, `isInteractable`, and `tileType` (enum)
- `GetTileWidth()` — getter for the width of the grid
- `GetTileHeight()` — getter for the height of the grid
- `IsValid()` — tests whether specific coordinates are within the bounds of the StateGrid
- `GenerateCellMap()` — generates the cell map based on parameters such as obstacle density, enemy count and types, and the current difficulty tier
- `AddCellType()` — helper that configures a new type of cell; parameters include name, description, and a symbol (e.g., a `char` for ASCII representation or a file path string for image-based tiles)

### Error Conditions
- Built-in parameter checkers prevent misuse, for example enforcing limits on width and height to avoid generating excessively large maps.
- An exception is thrown if there is insufficient memory to generate the tile map.
- An exception and a diagnostic message are thrown if an agent is able to pass through a tile it should not be able to.

### Expected Challenges
- Preventing player and enemy characters from walking through wall tiles requires proper hit-testing and boolean checkers.
- Generating unique and varied room shapes — not just rectangular blocks — with environmental features like foliage and obstacles.
- Ensuring procedurally generated obstacles do not block the path to the next area.

### Dependencies on Other Groups
This class likely won't rely on other groups' classes, though it may use `Random` for generating room features such as traps and environmental details. Collaboration with other groups will be needed to determine the GUI platform (wxWidgets, SDL, GTK) and tile representation format.

---

## StateGridPosition

**Lead:** Joey Hyun

### Description
Tracks the position and orientation of a single agent (player, enemy, or other) within a StateGrid. Stores coordinates and direction, and allows comparison of positions between agents. Supports collision detection, pathfinding, and relationship checks between agents and objects in the game world.

### Similar Classes
- `std::pair` — can store two coordinate values

### Key Functions
- `StateGridPosition(int x, int y, Orientation dir = ...)` — constructor to initialize position and optional facing direction
- `GetX()`, `GetY()` — getters for current coordinates
- `SetX()`, `SetY()` — setters for coordinates
- `Orientation()` — returns the current facing direction
- `Move()` — moves the position of the agent
- `bool operator==()` — compares whether two positions occupy the same cell

### Error Conditions
- Agent moving out of bounds.
- Tracking incorrect coordinates or direction.

### Expected Challenges
The main challenge will be ensuring this class works well with `StateGrid` for validity checking. The class only tracks position data, but `Move()` may need to verify whether the destination is valid within the grid. Accurately tracking each agent on the current map may also be complex.

### Dependencies on Other Groups
This class likely won't depend on another group's classes. Within the group, `DataFileManager` may be used to save position data.

---

## Vision for the Main Module

Group 15 is the **Generative World** group. The vision is a roguelike dungeon crawler built around procedurally generated rooms. Moving from one room to the next generates the next room. Planned room types include:

- Treasure rooms
- Monster rooms
- NPC rooms (merchants)
- Boss monster rooms
- Generic / save rooms

Treasure rooms (and potentially others) will generate their own loot: weapons of different rarities, types, and values; money or tradeable trinkets; and consumable items like health kits. Each room will also generate its own environment, including traps, path-blocking obstacles, and secret doors leading to better rooms.

As the player progresses they will move through 4 difficulty areas, each with distinct background visuals, monster types, and loot. A rough concept: starting in a forest, moving into a cave, then a dungeon, then a castle.

Additional systems planned:
- An NPC merchant that buys and sells items
- An inventory UI
- A player character

The team intends to use original artwork if possible, and has a member with drawing experience willing to contribute.

### Cross-Group Collaboration
- **Group 2** — classic agents needed for NPCs and the player character
- **Group 1** — AI agents module potentially needed for enemy monsters
- **Group 14** — collaboration needed on building an inventory system
- **Groups 17 & 18** — collaboration needed for graphics and on-screen rendering