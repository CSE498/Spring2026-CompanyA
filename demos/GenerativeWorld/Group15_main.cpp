/**
* This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A main file to demonstrate the basic functionality of the group 15,
 * generative world module.
 * @note Status: PROPOSAL
 * 
 * To run this demo:
 *    Step 1: follow the instructions in BUILD.md and CMAKE.md to build the executable
 *    Step 2: cd into /demo, and then run ./Group15_demo
 **/

// Include the modules that we will be using.
#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/Interfaces/TrashInterface.hpp"
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/Worlds/Dungeon/DungeonOne.hpp"

using namespace cse498;

int main()
{
    //MazeWorld world;
	DungeonOne world;
    world.AddAgent<PacingAgent>("Pacer 1").SetLocation(WorldPosition{3,1});
    world.AddAgent<PacingAgent>("Pacer 2").SetLocation(WorldPosition{6,1});
    world.AddAgent<PacingAgent>("Guard 1").SetHorizontal().SetLocation(WorldPosition{7,7});
    world.AddAgent<PacingAgent>("Guard 2").SetHorizontal().ToggleDirection().SetLocation(WorldPosition{8,8});
    world.AddAgent<TrashInterface>("Interface").SetSymbol('@').SetLocation(WorldPosition{1,1});

    world.Run();
}