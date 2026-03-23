#pragma once

#include "../../../core/WorldBase.hpp"
#include "../../../core/InterfaceBase.hpp"

#include <memory>
#include <concepts>

namespace cse498 {

class MockWorld : public WorldBase {
protected:
  enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, INTERACT, QUIT };

  size_t mFloorId; ///< Easy access to floor CellType ID.
  size_t mWallId;  ///< Easy access to wall CellType ID.
  std::unique_ptr<InterfaceBase> mInterface = nullptr;
  double actionTimer{0};

  void IncrementActionTimer(double millis) { actionTimer += millis; }

  template <std::derived_from<InterfaceBase> T>
  InterfaceBase & AddInterface(std::string interfaceName="None") {
    mInterface = std::make_unique<T>(agent_set.size(), interfaceName, *this);
    
    InterfaceBase & interfaceRef = *mInterface;
    ConfigAgent(*mInterface);

    if (mInterface->Initialize() == false) {
      std::cerr << "Failed to initialize interface" << std::endl;
    }
    return interfaceRef;
  }

  /// Provide the agent with movement actions.
  void ConfigAgent(AgentBase & agent) override {
    agent.AddAction("up", MOVE_UP);
    agent.AddAction("down", MOVE_DOWN);
    agent.AddAction("left", MOVE_LEFT);
    agent.AddAction("right", MOVE_RIGHT);
    agent.AddAction("interact", INTERACT);
    agent.AddAction("quit", QUIT);
  }

public:
  MockWorld() {
    mFloorId = main_grid.AddCellType("floor", "Floor that agents can walk on.", ' ');
    mWallId  = main_grid.AddCellType("wall",  "Impenetrable wall.",             '#');

    main_grid.Load(std::vector<std::string>{"#######################",
                                            "# #            ##     #",
                                            "# #  #  ######    ### #",
                                            "# #  #  #     #  #  # #",
                                            "# #  #  #  #  #  #  # #",
                                            "#    #     #     #    #",
                                            "##################  # #",
                                            "#                    ##",
                                            "#                    ##",
                                            "#  ####################",
                                            "#######################"} );
  }
  ~MockWorld() = default;

  /// Allow the agents to move around the maze.
  int DoAction(AgentBase & agent, size_t action_id) override {
    // Determine where the agent is trying to move.
    WorldPosition cur_position = agent.GetLocation().AsWorldPosition();
    WorldPosition new_position;
    switch (action_id) {
    case REMAIN_STILL: new_position = cur_position; break;
    case MOVE_UP:      new_position = cur_position.Up(); break;
    case MOVE_DOWN:    new_position = cur_position.Down(); break;
    case MOVE_LEFT:    new_position = cur_position.Left(); break;
    case MOVE_RIGHT:   new_position = cur_position.Right(); break;
    case INTERACT:     return true;
    case QUIT: 
    }

    // Don't let the agent move off the world or into a wall.
    if (!main_grid.IsValid(new_position)) { return false; }
    if (main_grid[new_position] == mWallId) { return false; }

    // Set the agent to its new postion.
    agent.SetLocation(new_position);

    return true;
  }

  void Run() override {
    if (actionTimer < 250) {
      mInterface->RenderFrame();
      return;
    }
    actionTimer = 0;
    size_t action_id = mInterface->SelectAction(main_grid);
    if (action_id == QUIT) Teardown();
    int result = DoAction(*mInterface, action_id);
    mInterface->SetActionResult(result);
    if (!mInterface->IsPaused()) {
      RunAgents();
    }
    mInterface->RenderFrame();
  }

  void Teardown() {
    mInterface.release();
    exit(0);
  }

};

}