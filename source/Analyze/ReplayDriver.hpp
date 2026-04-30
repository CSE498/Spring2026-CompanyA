#pragma once

#include <vector>

#include "../core/WorldBase.hpp"
#include "../core/AgentBase.hpp"

namespace cse498 {

class ReplayDriver {
private:
    // Stores the replay information for one agent.
    // Each agent has its ID, its saved actions, and the index of the next action to replay.
    struct AgentReplayState {
        size_t agentID = 0;
        std::vector<Action> actions;
        size_t nextAction = 0;
    };

    // The world that the replay driver is currently controlling.
    // This is a raw pointer because ReplayDriver does not own the world.
    WorldBase* mWorld = nullptr;

    // Replay state for every agent that has recorded actions.
    std::vector<AgentReplayState> mAgentReplays;

    // Tracks whether the replay is currently active.
    bool mRunning = false;

    // Returns true if at least one agent still has actions left to replay.
    [[nodiscard]] bool HasRemainingActions() const;
    
public:
    // Sets the world that should be replayed and clears old replay data.
    void SetWorld(WorldBase* world);

    // Builds replay state from the agents' action logs and begins replaying.
    void Start();

    // Stops the replay from continuing.
    void Stop();

    // Advances the replay by one step for each agent with remaining actions.
    void Step();

    // Returns whether the replay driver is currently running.
    [[nodiscard]] bool IsRunning() const;

    // Converts a recorded Action into the matching action ID for the given agent.
    size_t GetReplayActionForAgent(const AgentBase& agent, const Action& action) const;
};

} // namespace cse498