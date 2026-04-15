# Group 1: AI agents

This document describes the Group 1 components: the combat / loot demo world (AIWorld), the LootItem type used with it, three agent implementations, and the Group1_main entry point.

## Quick start

Build and run the Group 1 executable (see [BUILD.md](../BUILD.md) for project build instructions). The demo program is [source/Group1_main.cpp](../source/Group1_main.cpp): it constructs an AIWorld, adds a TrailblazerAgent and a LearningExplorerAgent, and calls world.Run().

## AIWorld and LootItem

AIWorld extends MazeWorld: same fixed maze grid (walls as #, floor walkable), plus:

- Agent combat stats per registered agent (AgentState: HP, max HP, attack, heal charges, score).
- Floor loot as LootItem instances (weapon or heal). Weapons increase attack when picked up; heal items grant charges consumed by the use_heal action.
- Enemies (EnemyState): positions on the grid, HP, attack. They are updated each world turn after all player-controlled agents act.
- Extra actions (beyond maze moves): pickup, use_heal, attack_up / attack_down / attack_left / attack_right.

Turn flow (Run): print state → each agent runs SelectAction then DoAction → optional step prompt → UpdateWorld moves/attacks with enemies → win/lose checks (all enemies dead, all agents dead, or turn limit).

Requirements for agents in AIWorld:

- Movement still uses MazeWorld’s action names: up, down, left, right (IDs 1–4 internally for moves in DoAction).
- Pickup, heal, and attacks only succeed when the world rules allow (e.g. item on cell, adjacent enemy for attack).

LootItem (ItemKind::Weapon or Heal, plus power) lives on the floor until picked up; consumed items no longer appear in GetFloorItems().

## Agent comparison

| Agent | Primary goal | Typical world | Notes |
|--------|----------------|---------------|--------|
| LearningExplorerAgent | Visit unexplored walkable cells | Any WorldBase with grid + wall/floor | Greedy exploration using visit counts and BFS toward nearest unvisited cell. No combat or loot. |
| EnemyAgent | Move toward another agent (“player”) | Any world with GetKnownAgents | Picks the first other known agent as target; scores cardinal moves by Manhattan distance. |
| TrailblazerAgent | Survive, gear up, defeat enemies | AIWorld strongly recommended | Uses dynamic_cast to AIWorld for items and enemies; GOAP-style plan over goals (heal, weapon, attack, explore). |

## LearningExplorerAgent

Purpose: Exploration-only behavior: prefer cells visited fewer times and use BFS to choose a step toward the nearest unvisited reachable walkable cell.

Requirements:

- Initialize() succeeds only if actions up, down, left, and right exist.

Not intended for: AIWorld combat (it does not use pickup, heal, or attack actions). It can still run inside AIWorld as a movement-only agent; see Group1_main below.

## EnemyAgent

Purpose: Simple “chaser”: each turn, among the four cardinal moves, choose the one that minimizes Manhattan distance to a target agent after the move. Invalid moves (out of bounds or into a wall cell) are heavily penalized.

Target selection: First other agent ID returned by world.GetKnownAgents(*this) that is not this; that agent’s grid position is treated as the player.

Requirements:

- Same four movement actions as LearningExplorerAgent.

Difference from LearningExplorerAgent: EnemyAgent optimizes distance to another entity; LearningExplorerAgent optimizes map coverage with no notion of a player.

## TrailblazerAgent

Purpose: Higher-level behavior for AIWorld: plan sequences such as moving to heals/weapons, picking them up, using heals when HP is low, moving adjacent to enemies, attacking, or exploring when other goals are unavailable.

Requirements:

- Initialize() always returns true (no mandatory action check in code), but meaningful behavior requires AIWorld so GetFloorItems(), GetEnemies(), and attack adjacency logic work.
- Expects extended actions registered by AIWorld::ConfigAgent: pickup, use_heal, attack_*, plus maze moves.

Difference from EnemyAgent: TrailblazerAgent is a full-game policy (items, HP, attacks); EnemyAgent only greedy-moves toward another agent on the grid.

## Group1_main

Demo with two agents on the same AIWorld:

1. Trailblazer — TrailblazerAgent at (1,1) with symbol 'T' (combat, loot, exploration).
2. Explorer — LearningExplorerAgent at (3,1) with symbol 'E' (BFS / visit-based exploration only).

Trailblazer is registered first, so it acts before Explorer each round. Then AIWorld::Run(). Use this file as a reference for wiring Group 1 types together.

## See also

- Headers: source/Worlds/AIWorld.hpp, source/Agents/*.hpp
- Base interfaces: source/core/AgentBase.hpp, source/core/WorldBase.hpp
- Maze base: source/Worlds/MazeWorld.hpp
