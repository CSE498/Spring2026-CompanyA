/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/Interfaces/TrashInterface.hpp"
#include "../../source/Worlds/Hub/Building.hpp"
#include "../../source/Worlds/Hub/InteractiveWorld.hpp"
#include "../../source/Worlds/Hub/InteractiveWorldInventory.hpp"
#include "../../source/Worlds/Hub/InteractiveWorldSaveManager.hpp"
#include "../../source/Worlds/Hub/ResourceProducer.hpp"
#include "../../source/Worlds/MazeWorld.hpp"

#include <iostream>
#include <memory>
#include <string>

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
      std::make_shared<Building>(1, "Lumber Yard", *world);
  lumberYard->SetSymbol('L');
  lumberYard->AddUpgrade(ItemType::Wood, 15);
  lumberYard->AddUpgrade(ItemType::Wood, 50);
  lumberYard->AddUpgrade(ItemType::Stone, 50);
  std::shared_ptr<Building> quarry =
      std::make_shared<Building>(2, "Quarry", *world);
  quarry->SetSymbol('Q');
  quarry->AddUpgrade(ItemType::Wood, 50);
  quarry->AddUpgrade(ItemType::Stone, 50);
  quarry->AddUpgrade(ItemType::Metal, 35);
  std::shared_ptr<Building> mine =
      std::make_shared<Building>(3, "Ore Mine", *world);
  mine->SetSymbol('M');
  mine->AddUpgrade(ItemType::Stone, 100);
  mine->AddUpgrade(ItemType::Metal, 50);
  mine->AddUpgrade(ItemType::Metal, 100);

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

  world->AddBuilding(lumberYard, WorldPosition{2, 1});
  world->AddBuilding(quarry, WorldPosition{5, 3});
  world->AddBuilding(mine, WorldPosition{8, 5});

  lumberYard->SetSymbol('L');
  quarry->SetSymbol('M');
  mine->SetSymbol('X');

  // Load save AFTER buildings/producers/world setup
  InteractiveWorldSaveManager saveManager;
  if (saveManager.Load(*world, "interactive_world_save.json")) {
    std::cout << "Loaded existing save file.\n";
  } else {
    std::cout << "No save file found. Starting new game.\n";
  }

  world->Run();
}
