/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "Agents/LearningExplorerAgent.hpp"
#include "Agents/PacingAgent.hpp"
#include "Interfaces/AIDebugInterface.hpp"
#include "Interfaces/TrashInterface.hpp"
#include "Worlds/MazeWorld.hpp"

using namespace cse498;

int main() {
  MazeWorld world;

  auto &explorer = world.AddAgent<LearningExplorerAgent>("Explorer");
  explorer.SetSymbol('E').SetLocation(WorldPosition{6, 1});

  world.AddAgent<PacingAgent>("Guard 1")
      .SetHorizontal()
      .SetSymbol('G')
      .SetLocation(WorldPosition{7, 7});

  auto &observer = world.AddAgent<AIDebugInterface>("Observer");

  observer.SetSymbol('O');
  observer.SetLocation(WorldPosition{1, 1});
  observer.SetMonitoredAgent(explorer.GetID());

  world.Run();
}
