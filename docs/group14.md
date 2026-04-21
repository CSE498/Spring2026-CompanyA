The interactive world currently functions as a resource logistics and building-management demo. As it exists in the Group 14 demo today, the world contains a central town hall, one resource bank per resource type, one resource spawn per resource type, and one upgradeable building per resource type.

The main gameplay loop currently visible in the demo is:

- Resources are generated into a `ResourceSpawn` by a `ResourceProducer`.
- A fetch agent moves from that spawn to the matching `ResourceBank`.
- A second fetch agent moves from that bank to the `TownHall` for final deposit.
- Each resource lane has an adjacent upgradeable building that controls that lane's production rate.

This is the current in-demo stand-in for the longer-term resource flow. The likely direction is that the building and bank responsibilities will be merged more tightly so the world does not need as many separate structures for each resource line.

## Current Demo State

The current Group 14 demo has:

- `1` `TownHall`
- `3` `ResourceSpawn`s
- `3` `ResourceBank`s
- `3` upgradeable `Building`s
- `6` `FetchAgent`s
- a farming agent present in the demo

The resource lanes are:

- Wood: `l` spawn -> `B` bank -> `T` town hall, with `L` as the upgradeable building
- Stone: `q` spawn -> `B` bank -> `T` town hall, with `Q` as the upgradeable building
- Metal: `m` spawn -> `B` bank -> `T` town hall, with `M` as the upgradeable building

The town hall is placed in the center of the map. The three resource spawns are placed in the corners farthest from the player start. Each bank is placed between its matching spawn and the town hall, and each upgradeable building is placed directly next to its matching bank.

## Demo Layout

Example layout of the current demo:

```text
Wood | Stone | Metal totals are printed above the map each turn
+-----------------------+
|#######################|
|#@                   l#|
|#                   1 #|
|#              LB     #|
|#             72      #|
|#          T          #|
|#      4       6      #|
|#    QB        MB     #|
|# 3                 5 #|
|#q                   m#|
|#######################|
+-----------------------+
```

Legend:

- `T` is the town hall
- `B` is a resource bank
- `L` is the lumber yard building and `l` is the wood spawn
- `Q` is the quarry building and `q` is the stone spawn
- `M` is the mine building and `m` is the metal spawn
- `1` and `2` are the wood-lane fetch agents
- `3` and `4` are the stone-lane fetch agents
- `5` and `6` are the metal-lane fetch agents
- `7` is the farming agent present in the demo

## Classes In Use

- **InteractiveWorld:** Main overworld class. Owns the map, agents, buildings, world inventory, and producer list.
- **InteractiveWorldInventory:** Stores the town hall's resource totals by `ItemType`.
- **TownHall:** Final deposit point for delivered resources. Writes into the world inventory.
- **ResourceSpawn:** Holds raw resources for one item type. This is where the first hauling leg picks up from.
- **ResourceBank:** Intermediate storage point for one hauling lane. Receives resources from the spawn-side fetch agent and supplies them to the town-hall-side fetch agent.
- **FetchAgent:** Moves between an origin point and a deposit point. In the current demo, fetch agents are assigned fixed routes for each hauling leg.
- **Building:** Upgradeable production structure. Each resource lane has one associated building.
- **ResourceProducer:** Generates one resource type over time into its matching spawn. The paired building level modifies the output rate.
- **InteractiveWorldSaveManager:** Saves and loads world state used by the interactive world demo.

## Current Limitation

The major current gap is building upgrades. The upgradeable buildings are present in the demo and are hooked into production, but the player-facing upgrade interaction path is not working yet in the current game flow.

The likely next step is to add a dedicated resource-management interaction point that can:

- spend wood, stone, and metal to upgrade buildings
- allow selling resources for gold
- gate town hall upgrades behind building progression
- apply a town hall upgrade bonus such as faster agents or a global production boost

This likely means the town hall will need to be treated more like a normal upgradeable building as that system is finalized.

## Manual Testing

The current demo entry point is:

- `demos/InteractiveWorld/Group14_main.cpp`

What to watch for in the current demo:

1. The map should load with the player in the top-left corner, the town hall in the center, and three resource lanes around it.
2. Each resource lane should have:
   a spawn in a corner, a bank between the spawn and town hall, and a matching upgradeable building next to the bank.
3. Agents `1` through `6` should continuously move resources along the two hauling legs for wood, stone, and metal.
4. The resource totals printed above the map should rise over time as resources are produced and delivered to the town hall.
5. Agent `7` is present in the demo, but the upgrade interaction flow is still pending.
