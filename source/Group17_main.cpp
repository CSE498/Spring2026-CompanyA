/**
 * @file Group17_main.cpp
 * @author Anagha Jammalamadaka
 * @brief Example driver demostrating GUI module integration with Agents module. Shows how
          Menu, GameView, and AgentBase work together in the game
 */

#include "core/Game.hpp"
#include "core/AgentBase.hpp"
#include "core/OverWorld.hpp"
#include "core/WorldGrid.hpp"
#include "core/WorldPosition.hpp"


#include <iostream>
#include <memory>


namespace cse498 {


class StubAgent : public AgentBase {
  public:

    StubAgent(size_t id, const std::string& name, const WorldBase& world) : AgentBase(id, name, world) {
      SetSymbol('@'); // represents player on screen
    }

    size_t SelectAction(const WorldGrid& /*grid*/) override{
      return 0; // 0 -> remain still
    }

    void Notify (const std::string& message, const std::string& msg_type = "none") override {
      std::cout << "[Agent Notification] type= " << msg_type << " message = "
      << message << std::endl;
    }


};

} // namespade cse498

int main() {

  std::cout << " === Agent + World Inegration ===" << std::endl;

  // create overworld
  cse498::OverWorld world;

  // create stub agent using overworld as context
  cse498::StubAgent player (1, "Player", world);

  std::cout << "Agent name:   " << player.GetName() << std::endl;
  std::cout << "Agent symbol: " << player.GetSymbol() << std::endl;

  // Demonstrate agent action system
  player.AddAction("up",    1);
  player.AddAction("down",  2);
  player.AddAction("left",  3);
  player.AddAction("right", 4);

  std::cout << "Has 'up' action:    " << (player.HasAction("up")    ? "yes" : "no") << std::endl;
  std::cout << "Has 'down' action:  " << (player.HasAction("down")  ? "yes" : "no") << std::endl;
  std::cout << "Has 'left' action:  " << (player.HasAction("left")  ? "yes" : "no") << std::endl;
  std::cout << "Has 'right' action: " << (player.HasAction("right") ? "yes" : "no") << std::endl;

  size_t action = player.SelectAction(world.GetGrid());
  std::cout << "Agent selected action ID: " << action << std::endl;

  player.Notify("Welcome to the overworld!", "system");
  player.Notify("Enemy spotted to the north!", "enemy_alert");

  std::cout << "\n=== GUI Module Initialization ===" << std::endl;
  cse498::Game game("Slay The Dungeon", 800, 600);
  if (!game.Initialize()) {
    std::cerr << "Failed to initialize game" << std::endl;
    return 1;
  }

  std::cout << "Starting game loop..." << std::endl;
  game.Run();
  return 0;
}


