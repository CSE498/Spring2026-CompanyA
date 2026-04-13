/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/Interfaces/TrashInterface.hpp"
#include "../../source/Worlds/Hub/InteractiveWorld.hpp"
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/Worlds/Hub/Building.hpp"
#include "../../source/Worlds/Hub/InteractiveWorldInventory.hpp"
#include "../../source/Worlds/Hub/NPC.hpp"
#include "../../source/Worlds/Hub/ResourceProducer.hpp"
#include "../../source/Worlds/Hub/InteractiveWorldSaveManager.hpp"

#include <string>
#include <iostream>
#include <memory>

using namespace cse498;

int main() {
  std::shared_ptr<InteractiveWorld> world =
      std::make_shared<InteractiveWorld>();
  
  // InteractiveWorldSaveManager saveManager;
  
  world->GetInventory().AddItem(ItemType::Wood, 10);
  world->GetInventory().AddItem(ItemType::Stone, 5);

  world->AddAgent<PacingAgent>("Pacer 1").SetLocation(WorldPosition{3, 1});
  world->AddAgent<PacingAgent>("Pacer 2").SetLocation(WorldPosition{6, 1});
  world->AddAgent<PacingAgent>("Guard 1").SetHorizontal().SetLocation(
      WorldPosition{7, 7});
  world->AddAgent<PacingAgent>("Guard 2")
      .SetHorizontal()
      .ToggleDirection()
      .SetLocation(WorldPosition{8, 8});
  world->AddAgent<TrashInterface>("Interface")
      .SetSymbol('@')
      .SetLocation(WorldPosition{1, 1});

  // Buildings
  std::shared_ptr<Building> lumberYard =
      std::make_shared<Building>("Lumber Yard");
  lumberYard->AddUpgrade(ItemType::Wood, 15);  // Level 1 -> 15 Wood
  lumberYard->AddUpgrade(ItemType::Wood, 50);  // Level 2 -> 50 Wood
  lumberYard->AddUpgrade(ItemType::Stone, 50); // Level 3 -> 50 Stone
  std::shared_ptr<Building> quarry = std::make_shared<Building>("Quarry");
  quarry->AddUpgrade(ItemType::Wood, 50);  // Level 1 -> 50 Wood
  quarry->AddUpgrade(ItemType::Stone, 50); // Level 2 -> 50 Stone
  quarry->AddUpgrade(ItemType::Metal, 35); // Level 3 -> 35 Metal
  std::shared_ptr<Building> mine = std::make_shared<Building>("Ore Mine");
  mine->AddUpgrade(ItemType::Stone, 100); // Level 1 -> 100 Stone
  mine->AddUpgrade(ItemType::Metal, 50);  // Level 2 -> 50 Metal
  mine->AddUpgrade(ItemType::Metal, 100); // Level 3 -> 100 Metal
  // NPCs
  std::shared_ptr<NPC> lumberjack = std::make_shared<NPC>(lumberYard);
  std::shared_ptr<NPC> mason = std::make_shared<NPC>(quarry);
  std::shared_ptr<NPC> miner = std::make_shared<NPC>(mine);
  // Resource Producers
  std::shared_ptr<ResourceProducer> woodProducer =
      std::make_shared<ResourceProducer>(lumberYard, world->GetInventory(),
                                         ItemType::Wood, 2);

  std::shared_ptr<ResourceProducer> stoneProducer =
      std::make_shared<ResourceProducer>(quarry, world->GetInventory(),
                                         ItemType::Stone, 1);

  std::shared_ptr<ResourceProducer> metalProducer =
      std::make_shared<ResourceProducer>(mine, world->GetInventory(),
                                         ItemType::Metal, 0.5);

  world->AddProducer(woodProducer);
  world->AddProducer(stoneProducer);
  world->AddProducer(metalProducer);
  world->AddNPC(lumberjack);
  world->AddNPC(mason);
  world->AddNPC(miner);

  lumberjack->SetPosition(WorldPosition{2, 1});
  mason->SetPosition(WorldPosition{5, 3});
  miner->SetPosition(WorldPosition{8, 5});

  lumberjack->SetSymbol('L');
  mason->SetSymbol('M');
  miner->SetSymbol('X');

  world->Run();

}
