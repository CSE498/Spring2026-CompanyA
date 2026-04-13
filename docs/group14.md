The interactive world (or overworld) will act as a resource and building management system. The player will gather resources in the dungeon and the overworld that can be used to upgrade buildings via NPC’s. Each building will have upgrades that will increase the output multiplier for a specific resource. Upgrading a building will be done by interacting with the NPC associated with the building. For example, If you want to upgrade the Lumber Yard to increase the production of Wood, you would go up to the Lumberjack NPC to access the next upgrade. Resources will be generated at some rate (# items / second). These resources will be used to upgrade the buildings and the base’s health.

## Classes

- **InteractiveWorld:** Main class for the world. Stores NPC’s, Player, Agents, Buildings ect…
- **InteractiveWorldInventory:** Holds the resources for the world. Resources are stored in a map<ItemType, size_t>. The 3 types of resources are Wood, Stone, and Metal
- **ResourceProducer:** Responsible for producing one kind of ItemType at a rate of ItemType/second. Because items are stored as ints, an accumulator float variable will count the fraction of a resource that has been produced. When the accumulator is >= 1.0 then the resource will add the accumulated amount as an int to the InteractiveWorldInventory  and keep any decimal in the accumulator. ResourceProducer will have an association to 1 Building. A building will modify the output rate of a ResourceProducer. The more a building is upgraded, the higher the output rate.
- **Building:** Modifies the output rate of 1 ResourceProducer. Has a vector of upgrades. The higher the level, the higher the rate multiplier for the ResourceProducer. Upgrading the building requires a specific quantity of a certain ItemType.
- **NPC:** The interaction point for the player to upgrade a Building. Based on some requirements the player will request an upgrade to the building that the NPC is associated with. NPCs do not have any agent behavior.
- **Interactive World Save Manager:** Saves and loads the world data such as Inventory, Building levels, base health, ect…

## Enums and Structs

- **ItemType (enum):** Represents a resource used for upgrading such as Wood, Stone, Metal.
- **BuildingUpgrade (struct):** Represents the item type needed for a building upgrade and the amount of said item needed.
- **UpgradeRejectionType (enum):** Why the upgrade was rejected


Testing with the `simple` executable
=====================

The Group 14 classes are all wired into the demo in `source/simple_main.cpp`.
That makes `simple` the easiest way to do a manual integration test for:

- `InteractiveWorld`
- `InteractiveWorldInventory`
- `Building`
- `NPC`
- `ResourceProducer`
- `InteractiveWorldSaveManager`

Controls in the demo:

- `W`, `A`, `S`, `D` move the player
- `E` interacts with an adjacent NPC
- `Q` quits and saves to `source/interactive_world_save.json`

What to do in `simple`
--------------------------

1. Confirm the overworld loads and shows the player `@` plus the three NPCs:
   `L`, `M`, and `X`.
2. At the starting position, press `E`. The player begins next to the
   lumberjack NPC, so this should exercise the NPC/building upgrade path.
   On a fresh run it should fail first because the world starts with only
   `10 Wood` and the first lumber yard upgrade costs `15 Wood`.
3. Watch the inventory line printed after each turn. Resource producers update
   every loop, so the wood/stone/metal totals should increase over time.
4. After enough turns have passed for wood to reach at least `15`, stand next
   to `L` and press `E` again. This should succeed and exercise:
   inventory use, NPC upgrade logic, and building level progression.
5. Press `Q` to quit. This should create or update
   `source/interactive_world_save.json`, which is the manual test for the save
   manager path currently used by the demo.
