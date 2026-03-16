#include "AIDebugInterface.hpp"

#include <limits>

namespace cse498 {

  AIDebugInterface::AIDebugInterface(size_t id,
                                     const std::string & name,
                                     const WorldBase & world)
    : InterfaceBase(id, name, world)
  { }

  bool AIDebugInterface::Initialize()
  {
    return true;
  }

  void AIDebugInterface::DrawGrid(const WorldGrid & grid,
                                  const std::vector<size_t> & item_ids,
                                  const std::vector<size_t> & agent_ids) const
  {
    std::vector<std::string> symbol_grid(grid.GetHeight());

    for (size_t y = 0; y < grid.GetHeight(); ++y) {
      symbol_grid[y].resize(grid.GetWidth());
      for (size_t x = 0; x < grid.GetWidth(); ++x) {
        symbol_grid[y][x] = grid.GetSymbol(WorldPosition{x, y});
      }
    }

    for (size_t id : item_ids) {
      const ItemBase & item = world.GetItem(id);
      if (!item.GetLocation().IsPosition()) continue;
      const WorldPosition pos = item.GetLocation().AsWorldPosition();
      symbol_grid[pos.CellY()][pos.CellX()] = '+';
    }

    for (size_t id : agent_ids) {
      const AgentBase & agent = world.GetAgent(id);
      if (!agent.GetLocation().IsPosition()) continue;
      const WorldPosition pos = agent.GetLocation().AsWorldPosition();
      symbol_grid[pos.CellY()][pos.CellX()] = agent.GetSymbol();
    }

    std::cout << '\n';
    std::cout << '+' << std::string(grid.GetWidth(), '-') << "+\n";
    for (const auto & row : symbol_grid) {
      std::cout << "|";
      for (char cell : row) std::cout << cell;
      std::cout << "|\n";
    }
    std::cout << '+' << std::string(grid.GetWidth(), '-') << "+\n";
  }

  void AIDebugInterface::PrintAgentSummary(const std::vector<size_t> & agent_ids) const
  {
    std::cout << "\nAgent summary:\n";
    for (size_t id : agent_ids) {
      const AgentBase & agent = world.GetAgent(id);
      std::cout << "  ["
                << agent.GetID()
                << "] "
                << agent.GetName()
                << " symbol='"
                << agent.GetSymbol()
                << "'";

      if (agent.GetLocation().IsPosition()) {
        const WorldPosition pos = agent.GetLocation().AsWorldPosition();
        std::cout << " pos=(" << pos.X() << ", " << pos.Y() << ")";
      }

      std::cout << " last_result=" << agent.GetActionResult();

      if (id == monitored_agent_id) {
        std::cout << "  <-- monitored";
      }

      std::cout << '\n';
    }
  }

  void AIDebugInterface::PauseIfNeeded() const
  {
    if (!step_mode) return;

    std::cout << "\nPress ENTER for next step...";
    std::cout.flush();

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (std::cin.fail()) {
      std::cin.clear();
    }
  }

  size_t AIDebugInterface::SelectAction(const WorldGrid & grid)
  {
    const auto item_ids = world.GetKnownItems(*this);
    const auto agent_ids = world.GetKnownAgents(*this);

    DrawGrid(grid, item_ids, agent_ids);

    if (show_stats) {
      PrintAgentSummary(agent_ids);
    }

    PauseIfNeeded();

    // This interface does not move; it only observes.
    return 0;
  }

} // namespace cse498