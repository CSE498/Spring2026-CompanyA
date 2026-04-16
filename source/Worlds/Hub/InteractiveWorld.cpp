/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief SDL-focused InteractiveWorld implementation.
 **/

#include "InteractiveWorld.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>

namespace cse498 {

namespace {

constexpr size_t kGuiWidth = 25;
constexpr size_t kGuiHeight = 18;

void FillGrid(WorldGrid &grid, size_t cell_id) {
  for (size_t y = 0; y < grid.GetHeight(); ++y) {
    for (size_t x = 0; x < grid.GetWidth(); ++x) {
      grid[x, y] = cell_id;
    }
  }
}

void PaintRect(WorldGrid &grid, size_t x0, size_t y0, size_t x1, size_t y1,
               size_t cell_id) {
  if (grid.GetWidth() == 0 || grid.GetHeight() == 0) {
    return;
  }

  const size_t max_x = std::min(x1, grid.GetWidth() - 1);
  const size_t max_y = std::min(y1, grid.GetHeight() - 1);

  for (size_t y = y0; y <= max_y; ++y) {
    for (size_t x = x0; x <= max_x; ++x) {
      grid[x, y] = cell_id;
    }
  }
}

bool IsAdjacentCardinal(const WorldPosition &a, const WorldPosition &b) {
  const int dx = static_cast<int>(a.CellX()) - static_cast<int>(b.CellX());
  const int dy = static_cast<int>(a.CellY()) - static_cast<int>(b.CellY());
  return std::abs(dx) + std::abs(dy) == 1;
}

} // namespace

InteractiveWorld::InteractiveWorld() { BuildDefaultRoom(); }

void InteractiveWorld::ConfigAgent(AgentBase &agent) {
  agent.AddAction("up", MOVE_UP);
  agent.AddAction("down", MOVE_DOWN);
  agent.AddAction("left", MOVE_LEFT);
  agent.AddAction("right", MOVE_RIGHT);
  agent.AddAction("interact", INTERACT);
}

void InteractiveWorld::BuildDefaultRoom() {
  m_inventory.Clear();
  m_buildings.clear();
  m_producers.clear();
  agent_set.clear();
  item_set.clear();
  mLastActionMessage.clear();
  mLastInventorySnapshot.clear();
  mPlayerPosition = WorldPosition{1, 1};

  main_grid = WorldGrid(23, 11);
  grass_id =
      main_grid.AddCellType("floor", "Floor that agents can walk on.", ' ');
  wall_left_id = main_grid.AddCellType("wall", "Impenetrable wall.", '#');

  main_grid.Load(std::vector<std::string>{
      "#######################", "#                     #",
      "#                     #", "#                     #",
      "#                     #", "#                     #",
      "#                     #", "#                     #",
      "#                     #", "#                     #",
      "#######################"});
}

void InteractiveWorld::BuildGuiTerrain() {
  main_grid = WorldGrid(kGuiWidth, kGuiHeight);

  grass_id = main_grid.AddCellType("grass", "Hub grass.", '.');
  grass_flowers_id =
      main_grid.AddCellType("grass_flowers", "Flower patch.", 'f');
  grass_bones_id =
      main_grid.AddCellType("grass_bones", "Worn mining soil.", 'b');
  grass_mud_id = main_grid.AddCellType("grass_mud", "Muddy ground.", 'm');
  grass_rock_id = main_grid.AddCellType("grass_rock", "Rocky ground.", 'r');
  entrance_id = main_grid.AddCellType("entrance", "Gate to the dungeon.", 'E');
  wall_left_id = main_grid.AddCellType("wall_left", "Left wall.", 'L');
  wall_right_id = main_grid.AddCellType("wall_right", "Right wall.", 'R');
  wall_top_id = main_grid.AddCellType("wall_top", "Top wall.", 'U');
  wall_bottom_id = main_grid.AddCellType("wall_bottom", "Bottom wall.", 'B');
  wall_corner_id = main_grid.AddCellType("wall_corner", "Corner wall.", 'C');
  lumber_yard_id =
      main_grid.AddCellType("lumber_yard", "Lumber yard building.", 'Y');
  quarry_id = main_grid.AddCellType("quarry", "Quarry building.", 'Q');
  ore_mine_id = main_grid.AddCellType("ore_mine", "Ore mine building.", 'O');

  FillGrid(main_grid, grass_id);

  for (size_t x = 1; x + 1 < main_grid.GetWidth(); ++x) {
    main_grid[x, 0] = wall_top_id;
    main_grid[x, main_grid.GetHeight() - 1] = wall_bottom_id;
  }
  for (size_t y = 1; y + 1 < main_grid.GetHeight(); ++y) {
    main_grid[0, y] = wall_left_id;
    main_grid[main_grid.GetWidth() - 1, y] = wall_right_id;
  }
  main_grid[0, 0] = wall_corner_id;
  main_grid[main_grid.GetWidth() - 1, 0] = wall_corner_id;
  main_grid[0, main_grid.GetHeight() - 1] = wall_corner_id;
  main_grid[main_grid.GetWidth() - 1, main_grid.GetHeight() - 1] =
      wall_corner_id;

  PaintRect(main_grid, 1, 1, main_grid.GetWidth() - 2, 4, grass_rock_id);
  PaintRect(main_grid, 1, 13, main_grid.GetWidth() - 2,
            main_grid.GetHeight() - 2, grass_mud_id);

  PaintRect(main_grid, 3, 5, 7, 8, grass_flowers_id);
  PaintRect(main_grid, 10, 7, 14, 10, grass_flowers_id);
  PaintRect(main_grid, 16, 11, 20, 14, grass_bones_id);

  PaintRect(main_grid, 11, 1, 13, main_grid.GetHeight() - 2, grass_id);
  PaintRect(main_grid, 2, 9, 22, 10, grass_id);
  PaintRect(main_grid, 4, 6, 11, 7, grass_id);
  PaintRect(main_grid, 13, 4, 14, 9, grass_id);
  PaintRect(main_grid, 13, 13, 18, 13, grass_id);

  main_grid[12, 2] = entrance_id;
  mPlayerPosition = WorldPosition{12, 11};
}

void InteractiveWorld::SetupGuiBuildings() {
  m_inventory.Clear();
  m_inventory.AddItem(ItemType::Wood, 20);
  m_inventory.AddItem(ItemType::Stone, 12);
  m_inventory.AddItem(ItemType::Metal, 4);

  auto lumber_yard = std::make_shared<Building>(1, "Lumber Yard", *this);
  lumber_yard->SetSymbol('L');
  lumber_yard->AddUpgrade(ItemType::Wood, 15);
  lumber_yard->AddUpgrade(ItemType::Wood, 35);
  lumber_yard->AddUpgrade(ItemType::Stone, 45);

  auto quarry = std::make_shared<Building>(2, "Quarry", *this);
  quarry->SetSymbol('Q');
  quarry->AddUpgrade(ItemType::Wood, 20);
  quarry->AddUpgrade(ItemType::Stone, 30);
  quarry->AddUpgrade(ItemType::Metal, 35);

  auto ore_mine = std::make_shared<Building>(3, "Ore Mine", *this);
  ore_mine->SetSymbol('M');
  ore_mine->AddUpgrade(ItemType::Stone, 25);
  ore_mine->AddUpgrade(ItemType::Metal, 35);
  ore_mine->AddUpgrade(ItemType::Metal, 55);

  auto wood_producer = std::make_shared<ResourceProducer>(
      lumber_yard, m_inventory, ItemType::Wood, 1.6f);
  auto stone_producer = std::make_shared<ResourceProducer>(
      quarry, m_inventory, ItemType::Stone, 1.0f);
  auto metal_producer = std::make_shared<ResourceProducer>(
      ore_mine, m_inventory, ItemType::Metal, 0.45f);

  AddProducer(wood_producer);
  AddProducer(stone_producer);
  AddProducer(metal_producer);

  AddBuilding(lumber_yard, WorldPosition{5, 6}, lumber_yard_id);
  AddBuilding(quarry, WorldPosition{12, 4}, quarry_id);
  AddBuilding(ore_mine, WorldPosition{18, 13}, ore_mine_id);
}

void InteractiveWorld::LoadGuiLayout() {
  m_buildings.clear();
  m_producers.clear();
  agent_set.clear();
  item_set.clear();
  mLastActionMessage.clear();
  mLastInventorySnapshot.clear();

  BuildGuiTerrain();
  SetupGuiBuildings();

  SetActionMessage("InteractiveWorld hub ready. Use WASD to move and E beside "
                   "a building to upgrade.",
                   true);
  PrintInventoryIfChanged(true);
}

void InteractiveWorld::UpdateWorld() {
  for (const auto &producer : m_producers) {
    producer->Update();
  }

  PrintInventoryIfChanged();
}

void InteractiveWorld::AdvanceSimulation() { UpdateWorld(); }

void InteractiveWorld::PrintInventoryIfChanged(bool force) {
  std::ostringstream output;
  output << m_inventory.GetAmount(ItemType::Wood) << ' '
         << ItemTypeToString(ItemType::Wood) << " | "
         << m_inventory.GetAmount(ItemType::Stone) << ' '
         << ItemTypeToString(ItemType::Stone) << " | "
         << m_inventory.GetAmount(ItemType::Metal) << ' '
         << ItemTypeToString(ItemType::Metal);

  const std::string snapshot = output.str();
  if (!force && snapshot == mLastInventorySnapshot) {
    return;
  }

  mLastInventorySnapshot = snapshot;
  std::cout << snapshot << std::endl;
}

void InteractiveWorld::SetActionMessage(std::string message,
                                        bool print_to_console) {
  mLastActionMessage = std::move(message);
  if (print_to_console && !mLastActionMessage.empty()) {
    std::cout << mLastActionMessage << std::endl;
  }
}

bool InteractiveWorld::IsBlockedTile(const WorldPosition &position) const {
  if (!main_grid.IsValid(position)) {
    return true;
  }

  const size_t cell_id = main_grid[position];
  return cell_id == wall_left_id || cell_id == wall_right_id ||
         cell_id == wall_top_id || cell_id == wall_bottom_id ||
         cell_id == wall_corner_id || cell_id == lumber_yard_id ||
         cell_id == quarry_id || cell_id == ore_mine_id;
}

bool InteractiveWorld::IsWalkable(const WorldPosition &position) const {
  return !IsBlockedTile(position);
}

bool InteractiveWorld::IsBuildingAt(const WorldPosition &position) const {
  return FindBuildingIndexAt(position).has_value();
}

bool InteractiveWorld::IsPlayerAt(const WorldPosition &position) const {
  return mPlayerPosition == position;
}

bool InteractiveWorld::IsAgentAt(const WorldPosition &position,
                                 const AgentBase *ignore) const {
  for (const auto &agent_ptr : agent_set) {
    if (!agent_ptr) {
      continue;
    }
    if (ignore != nullptr && agent_ptr.get() == ignore) {
      continue;
    }
    if (!agent_ptr->GetLocation().IsPosition()) {
      continue;
    }
    if (agent_ptr->GetLocation().AsWorldPosition() == position) {
      return true;
    }
  }
  return false;
}

std::optional<size_t>
InteractiveWorld::FindBuildingIndexAt(const WorldPosition &position) const {
  for (size_t i = 0; i < m_buildings.size(); ++i) {
    const auto &building = m_buildings[i].building;
    if (!building || !building->GetLocation().IsPosition()) {
      continue;
    }
    if (building->GetLocation().AsWorldPosition() == position) {
      return i;
    }
  }
  return std::nullopt;
}

std::optional<size_t> InteractiveWorld::FindAdjacentBuildingIndex() const {
  for (size_t i = 0; i < m_buildings.size(); ++i) {
    const auto &building = m_buildings[i].building;
    if (!building || !building->GetLocation().IsPosition()) {
      continue;
    }
    if (IsAdjacentCardinal(mPlayerPosition,
                           building->GetLocation().AsWorldPosition())) {
      return i;
    }
  }
  return std::nullopt;
}

bool InteractiveWorld::TryUpgradeBuilding(size_t building_index) {
  assert(building_index < m_buildings.size());
  auto &building_state = m_buildings[building_index];
  auto &building = building_state.building;
  assert(building);

  const auto next_upgrade = building->GetNextUpgradeInfo();
  if (!next_upgrade.has_value()) {
    SetActionMessage(building->GetName() + " is already fully upgraded.");
    return false;
  }

  const auto required_item = next_upgrade->item;
  const auto required_quantity =
      static_cast<InteractiveWorldInventory::ItemCount>(next_upgrade->quantity);
  const auto available = m_inventory.GetAmount(required_item);

  auto result = building->Upgrade(required_item, static_cast<int>(available));
  if (!result.has_value()) {
    std::ostringstream message;
    message << building->GetName() << " upgrade failed: "
            << Building::UpgradeRejectionTypeToString(result.error());
    SetActionMessage(message.str());
    return false;
  }

  const bool removed = m_inventory.RemoveItem(required_item, required_quantity);
  assert(removed && "Upgrade validation passed but inventory removal failed.");

  if (building_state.producer) {
    building_state.producer->CalculateRate();
  }

  std::ostringstream message;
  message << building->GetName() << " upgraded to level "
          << building->GetCurrentLevel();
  if (building_state.producer) {
    message << ". Production is now " << building_state.producer->GetRate()
            << " " << ItemTypeToString(building_state.producer->GetOutputType())
            << "/sec";
  }

  SetActionMessage(message.str());
  PrintInventoryIfChanged(true);
  return true;
}

bool InteractiveWorld::TryMovePlayer(PlayerMove move) {
  WorldPosition next_position = mPlayerPosition;

  switch (move) {
  case PlayerMove::Up:
    next_position = mPlayerPosition.Up();
    break;
  case PlayerMove::Down:
    next_position = mPlayerPosition.Down();
    break;
  case PlayerMove::Left:
    next_position = mPlayerPosition.Left();
    break;
  case PlayerMove::Right:
    next_position = mPlayerPosition.Right();
    break;
  }

  if (!IsWalkable(next_position)) {
    SetActionMessage("You cannot move there.", false);
    return false;
  }

  if (IsAgentAt(next_position)) {
    SetActionMessage("That space is occupied.", false);
    return false;
  }

  mPlayerPosition = next_position;
  return true;
}

bool InteractiveWorld::Interact() {
  const auto building_index = FindAdjacentBuildingIndex();
  if (!building_index.has_value()) {
    SetActionMessage("No building is close enough to interact with.");
    return false;
  }

  return TryUpgradeBuilding(*building_index);
}

int InteractiveWorld::DoAction(AgentBase &agent, size_t action_id) {
  WorldPosition current_position = agent.GetLocation().AsWorldPosition();
  WorldPosition next_position = current_position;

  switch (action_id) {
  case REMAIN_STILL:
    return true;
  case MOVE_UP:
    next_position = current_position.Up();
    break;
  case MOVE_DOWN:
    next_position = current_position.Down();
    break;
  case MOVE_LEFT:
    next_position = current_position.Left();
    break;
  case MOVE_RIGHT:
    next_position = current_position.Right();
    break;
  case INTERACT:
    return true;
  default:
    return false;
  }

  if (!IsWalkable(next_position)) {
    return false;
  }

  if (IsPlayerAt(next_position) || IsAgentAt(next_position, &agent)) {
    return false;
  }

  agent.SetLocation(next_position);
  return true;
}

std::vector<std::pair<WorldPosition, char>>
InteractiveWorld::GetOverlaySymbols() const {
  std::vector<std::pair<WorldPosition, char>> symbols;
  for (const auto &building_state : m_buildings) {
    const auto &building = building_state.building;
    if (!building || !building->GetLocation().IsPosition()) {
      continue;
    }
    symbols.emplace_back(building->GetLocation().AsWorldPosition(),
                         building->GetSymbol());
  }
  return symbols;
}

void InteractiveWorld::AddProducer(std::shared_ptr<ResourceProducer> producer) {
  if (!producer) {
    return;
  }

  m_producers.push_back(producer);

  const auto &building = producer->GetBuilding();
  if (!building) {
    return;
  }

  for (auto &building_state : m_buildings) {
    if (building_state.building == building) {
      building_state.producer = producer;
      return;
    }
  }
}

void InteractiveWorld::AddBuilding(std::shared_ptr<Building> building,
                                   WorldPosition position, size_t tile_id) {
  if (!building) {
    return;
  }

  building->SetLocation(Location(position));

  BuildingState state;
  state.building = building;
  state.tile_id = tile_id;

  for (const auto &producer : m_producers) {
    if (producer && producer->GetBuilding() == building) {
      state.producer = producer;
      break;
    }
  }

  if (tile_id != 0 && main_grid.IsValid(position)) {
    main_grid[position] = tile_id;
  }

  m_buildings.push_back(std::move(state));
}

} // namespace cse498
