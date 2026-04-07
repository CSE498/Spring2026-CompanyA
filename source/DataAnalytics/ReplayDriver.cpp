#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "../Interfaces/TrashInterface.hpp"
#include "../Worlds/MazeWorld.hpp"
#include "ReplayDriver.hpp"

// Helper function to Replay a move
void cse498::ReplayDriver::ReplayAction(const cse498::Action &action,
                                        cse498::TrashInterface &agent,
                                        MazeWorld &world) {
  char move = 'x';

  if (action.Position.X() > action.NewPosition.X())
    move = 'a';
  else if (action.Position.X() < action.NewPosition.X())
    move = 'd';
  else if (action.Position.Y() > action.NewPosition.Y())
    move = 'w';
  else if (action.Position.Y() < action.NewPosition.Y())
    move = 's';
  else
    return;

  std::istringstream fake_input(std::string(1, move) + "\n");
  std::streambuf *original_buf = std::cin.rdbuf();
  std::cin.rdbuf(fake_input.rdbuf());

  size_t action_id = agent.SelectAction(world.GetGrid());
  int result = world.DoAction(agent, action_id);
  agent.SetActionResult(result);

  std::cin.rdbuf(original_buf);
  world.UpdateWorld();

  std::this_thread::sleep_for(std::chrono::milliseconds(m_delay));

}

// Replays actions that occurred within a specific time range
void cse498::ReplayDriver::ReplayByTimeRange(double start_time,
                                             double end_time) {
  MazeWorld world;
  TrashInterface &agent = world.AddAgent<TrashInterface>("Interface");
  agent.SetSymbol('@');
  agent.SetLocation(WorldPosition{1, 1});
  std::vector<Action> actions_in_range =
      m_actionLog->GetActionRange(start_time, end_time);
  for (auto &action : actions_in_range) {
    ReplayAction(action, agent, world);
  }
}

void cse498::ReplayDriver::ReplayFullGame() {
  MazeWorld world;
  TrashInterface &agent = world.AddAgent<TrashInterface>("Interface");
  agent.SetSymbol('@');
  agent.SetLocation(WorldPosition{1, 1});
  for (auto &action : m_actionLog->GetActions()) {
    ReplayAction(action, agent, world);
  }
}