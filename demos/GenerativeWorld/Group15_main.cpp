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

/** @brief Include the modules used by this demo executable. */
#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/Interfaces/TrashInterface.hpp"
#include "../../source/tools/DataFileManager.hpp"
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/Worlds/Dungeon/DungeonWorld.hpp"

#include <iostream>
#include <string>

using namespace cse498;

///Commented this out for the time being
// int main()
// {
//     //MazeWorld world;
// 	    DungeonWorld world;
//      world.AddAgent<PacingAgent>("Pacer 1").SetLocation(WorldPosition{3,1});
//      world.AddAgent<PacingAgent>("Pacer 2").SetLocation(WorldPosition{6,1});
//      world.AddAgent<PacingAgent>("Guard 1").SetHorizontal().SetLocation(WorldPosition{7,7});
//      world.AddAgent<PacingAgent>("Guard 2").SetHorizontal().ToggleDirection().SetLocation(WorldPosition{8,8});
//      world.AddAgent<TrashInterface>("Interface").SetSymbol('@').SetLocation(WorldPosition{1,1});

//      world.Run();
// }


/**
 * @brief Populate the demo world with default agents and interfaces.
 * @param world World instance to seed with entities.
 */
static void PopulateDemoWorld(WorldBase & world)
{
    world.AddAgent<PacingAgent>("Pacer 1").SetLocation(WorldPosition{3,1});
    world.AddAgent<PacingAgent>("Pacer 2").SetLocation(WorldPosition{6,1});
    world.AddAgent<PacingAgent>("Guard 1").SetHorizontal().SetLocation(WorldPosition{7,7});
    world.AddAgent<PacingAgent>("Guard 2").SetHorizontal().ToggleDirection().SetLocation(WorldPosition{8,8});
    world.AddAgent<TrashInterface>("Interface").SetSymbol('@').SetLocation(WorldPosition{1,1});
}


/**
 * @brief Run the Group 15 generative world demo.
 * @param argc Number of command-line arguments.
 * @param argv Command-line arguments. Use "load" to load JSON data.
 * @return int Process exit code.
 */
int main(int argc, char* argv[])
{
    const std::string json_path = "demo/Group15_demo.json";
    bool should_load_data = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "load") {
            should_load_data = true;
        }
    }

    auto world = std::make_unique<DungeonWorld>();
    PopulateDemoWorld(*world);

    DataFileManager data_manager(json_path, std::move(world));

    if (should_load_data) {
        try {
            data_manager.LoadData();
            std::cout << "Loaded JSON data from: " << json_path << std::endl;
        } catch (const std::exception & err) {
            std::cerr << "Failed to load JSON data from: " << json_path << " - " << err.what() << std::endl;
        }
    }

    data_manager.GetWorld().Run();
    data_manager.Update();

    std::cout << "Saved JSON data to: " << json_path << std::endl;
}
