The interactive world is now a GUI-first resource logistics and building-management demo. It contains a central town hall, resource spawns, upgradeable production buildings, autonomous fetch agents, and a resource-management NPC used for upgrades, selling resources, and unlocking later resource lanes.

All further game testing for InteractiveWorld should be run through the GUI demo, not the older text-only Group 14 demo. The current gameplay behavior, assets, autonomous movement timing, and resource visuals are all represented in the GUI path.

## Current Demo State

The GUI InteractiveWorld currently has:

- `1` `TownHall`
- `3` `ResourceSpawn`s: wood, stone, and metal
- `3` upgradeable `Building`s: lumber yard, quarry, and mine
- `6` `FetchAgent`s, two per resource lane
- `1` `ResourceManagementAgent`

The resource lanes are:

- Wood: wood spawn -> lumber yard -> town hall
- Stone: stone spawn -> quarry -> town hall
- Metal: metal spawn -> mine -> town hall

The wood lane starts active. The quarry and mine lanes start locked and are hidden in the GUI until purchased from the resource-management menu. Their producers are also deferred until unlock, so locked lanes do not accumulate resources before the player buys them.

## Resource Production

Resources are produced in bursts instead of continuously. Nothing is added between burst intervals.

Current base production timing:

- Wood: `10` every `5` seconds
- Stone: `5` every `10` seconds
- Metal: `5` every `15` seconds

Building upgrades still modify production through the existing building rate modifier. Upgraded buildings use different generated sprites so upgrade state is visible in the world.

## Resource Visual States

Resource nodes use generated three-state assets:

- Empty: no collectible resources
- Partial: resources are present but not ready to collect
- Full: resources are ready for fetch agents or manual collection

Fetch agents do not collect from a spawn until the spawn reaches the full visual state. This makes resource growth/presence visible before the node is harvested.

## Autonomous Agents

Overworld non-player agents move on their own timed update. They no longer wait for player turns. The dungeon remains turn-driven by the existing dungeon update path.

Fetch agents are active only for unlocked lanes. Inactive lane fetchers are skipped by the renderer until their lane is purchased.

## GUI Controls

- `WASD`: move the player
- `E`: interact with nearby NPCs/buildings/resource systems
- `X`: debug-fill the InteractiveWorld inventory with wood, stone, and metal for upgrade testing
- Resource-management menu:
  - left/right: change tabs
  - up/down: move selection
  - enter: confirm
  - `E`: close

The overworld inventory hotbar/backpack overlay is intentionally hidden in InteractiveWorld. The world resource totals remain visible through the InteractiveWorld resource UI, and the dungeon inventory UI is unchanged.

## Classes In Use

- **InteractiveWorld:** Main overworld class. Owns the map, agents, buildings, world inventory, and producer list.
- **InteractiveWorldInventory:** Stores town hall/world resource totals by `ItemType`.
- **TownHall:** Final deposit point for delivered resources.
- **ResourceSpawn:** Holds raw resources for one item type and exposes generated visual state through quantity thresholds.
- **Building:** Upgradeable production structure. Each building also stores lane resources.
- **FetchAgent:** Autonomous hauling agent assigned to a fixed origin/deposit route. Spawn pickup waits until the spawn reaches full state.
- **ResourceManagementAgent:** GUI menu target for building upgrades, resource selling, and lane unlocks.
- **ResourceProducer:** Adds burst resources into a matching spawn at resource-specific intervals.
- **InteractiveWorldSaveManager:** Saves and loads InteractiveWorld inventory, building levels, resource-manager gold, and lane unlock state.

## Manual Testing

Use the GUI demo for InteractiveWorld testing:

```bash
cmake --build build --target group17_demo
./demos/group17_demo
```

What to verify:

1. The overworld loads with the town hall, lumber yard, wood spawn, wood fetchers, player, and resource manager visible.
2. Quarry and mine lane objects are not visible until their lanes are purchased.
3. Wood resources appear over time in burst states: empty, partial, then full.
4. Fetch agents collect only after a resource reaches the full visual state.
5. Non-player overworld agents move without requiring player turns.
6. Resource totals increase after fetchers deliver resources to the town hall.
7. The resource-management menu can unlock quarry and mine lanes.
8. Unlocked quarry and mine lanes appear and start producing on their configured burst timers.
9. Building upgrades spend resources, change building level, change building sprite, and increase production output.
10. Pressing `X` in InteractiveWorld fills resources for upgrade testing.
