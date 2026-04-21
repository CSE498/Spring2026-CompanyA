The interactive world functions as a resource logistics and building-management demo. The Group 14 demo contains a central town hall, one resource spawn per resource type, and one upgradeable production building per resource type. Each production building also serves as the intermediate storage point for its lane.

The main gameplay loop currently visible in the demo is:

- Resources are generated into a `ResourceSpawn` by a `ResourceProducer`.
- A fetch agent moves from that spawn to the matching production building.
- A second fetch agent moves from that building to the `TownHall` for final deposit.
- Each production building controls that lane's production rate through its upgrade level.

## Current Demo State

The current Group 14 demo has:

- `1` `TownHall`
- `3` `ResourceSpawn`s
- `3` upgradeable `Building`s that store lane resources
- `6` `FetchAgent`s
- `1` `ResourceManagementAgent`

The resource lanes are:

- Wood: `l` spawn -> `L` lumber yard -> `T` town hall
- Stone: `q` spawn -> `Q` quarry -> `T` town hall
- Metal: `m` spawn -> `M` mine -> `T` town hall

The town hall is placed in the center of the map. The three resource spawns are placed in the corners farthest from the player start. Each production building is placed between its matching spawn and the town hall, and the two fetch agents assigned to that lane shuttle resources across the two hauling legs.

## Demo Layout

Example layout of the current demo:

```text
Wood | Stone | Metal totals are printed above the map each turn
+-----------------------+
|#######################|
|#@                   l#|
|#                   1 #|
|#              L      #|
|#             72      #|
|#          T          #|
|#      4       6      #|
|#    Q         M      #|
|# 3                 5 #|
|#q                   m#|
|#######################|
+-----------------------+
```

Legend:

- `T` is the town hall
- `L` is the lumber yard building and `l` is the wood spawn
- `Q` is the quarry building and `q` is the stone spawn
- `M` is the mine building and `m` is the metal spawn
- `1` and `2` are the wood-lane fetch agents
- `3` and `4` are the stone-lane fetch agents
- `5` and `6` are the metal-lane fetch agents
- `7` is the resource-management agent

## Classes In Use

- **InteractiveWorld:** Main overworld class. Owns the map, agents, buildings, world inventory, and producer list.
- **InteractiveWorldInventory:** Stores the town hall's resource totals by `ItemType`.
- **TownHall:** Final deposit point for delivered resources. Writes into the world inventory.
- **ResourceSpawn:** Holds raw resources for one item type. This is where the first hauling leg picks up from.
- **Building:** Upgradeable production structure. Each building also stores lane resources.
- **FetchAgent:** Moves between an origin point and a deposit point. Fetch agents are assigned fixed routes for each hauling leg.
- **ResourceManagementAgent:** Central interaction point for upgrading buildings and selling stored resources for gold.
- **ResourceProducer:** Generates one resource type over time into its matching spawn. The paired building level modifies the output rate.
- **InteractiveWorldSaveManager:** Saves and loads world state used by the interactive world demo.

## Next Steps

The likely next step is to extend that resource-management interaction point so it can:

- spend wood, stone, and metal on building upgrades
- support richer upgrade rules than the current one-resource-per-upgrade model
- optionally handle selling from building-local storage if the design keeps building banks player-facing
- centralize future town hall upgrade logic

On that note, TownHall will be extended to have an upgrade path, changing/affecting game state in some way

## Manual Testing

The current demo entry point is:

- `demos/InteractiveWorld/Group14_main.cpp`

What to watch for in the current demo:

1. The map should load with the player/interface agent in the top-left corner, the town hall in the center, and three resource lanes around it.
2. Each resource lane should have a spawn in a corner, a production building between the spawn and the town hall, and two fetch agents moving along the lane.
3. Agents `1` through `6` should continuously move resources along the two hauling legs for wood, stone, and metal.
4. The resource totals printed above the map should rise over time as resources are produced and delivered to the town hall.
5. Agent `7` should open the resource-management menu when the player stands next to it and presses `E`.
6. The resource-management menu should allow selling stored wood, stone, and metal for gold.
7. The resource-management menu should allow upgrading any registered building independently when enough resources are available.
