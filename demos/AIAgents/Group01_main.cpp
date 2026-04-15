/**
 * @file Group1_main.cpp
 * @brief Group 1 demo entry: AIWorld + TrailblazerAgent +
 *LearningExplorerAgent, then Run().
 *
 * @details Spawns Trailblazer (GOAP/combat) at (1,1) as 'T', and Explorer (BFS
 *          coverage) at (3,1) as 'E'. For narrative docs, see docs/Group1.md.
 *
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 **/

// Include the modules that we will be using.
#include "../../source/Agents/AI/EnemyAgent.hpp"
#include "../../source/Agents/AI/LearningExplorerAgent.hpp"
#include "../../source/Agents/AI/SmartEnemyAgent.hpp"
#include "../../source/Agents/AI/TrailblazerAgent.hpp"
#include "../../source/Worlds/DemoG1/AIWorld.hpp"

using namespace cse498;

int main() {
    AIWorld world;

    world.AddAgent<TrailblazerAgent>("Trailblazer").SetSymbol('T').SetLocation(WorldPosition{1, 1});

    world.AddAgent<LearningExplorerAgent>("Explorer").SetSymbol('E').SetLocation(WorldPosition{3, 1});

    world.AddAgent<SmartEnemyAgent>("Boss").SetSymbol('B').SetLocation(WorldPosition{7, 1});

    world.AddAgent<EnemyAgent>("Enemy").SetSymbol('X').SetLocation(WorldPosition{8, 1});

    world.Run();
}
