#include "ReplayDriver.hpp"

namespace cse498 {

void ReplayDriver::SetWorld(WorldBase* world) {
    // Store the world that this replay driver will control.
    // This does not start replay yet; it only prepares the driver with a world.
    mWorld = world;

    // Clear any replay data from a previous world or previous replay.
    mAgentReplays.clear();

    // Make sure replay is stopped until Start() is called.
    mRunning = false;
}

void ReplayDriver::Start() {
    // Reset replay state every time a new replay starts.
    mAgentReplays.clear();
    mRunning = false;

    // If no world has been assigned, there is nothing to replay.
    if (mWorld == nullptr) {
        return;
    }

    // Go through every agent in the world and collect its recorded actions.
    for (size_t i = 0; i < mWorld->GetNumAgents(); ++i) {
        AgentBase& agent = mWorld->GetAgentByIndex(i);

        // Get the actions that were logged during normal gameplay.
        const std::vector<Action>& liveActions = agent.GetActionLog().GetActions();

        // Agents with no logged actions do not need replay state.
        if (liveActions.empty()) {
            continue;
        }

        // Create a replay state object for this agent.
        // It stores the agent ID, the actions to replay, and which action is next.
        AgentReplayState replayState;
        replayState.agentID = agent.GetID();
        replayState.actions = liveActions;
        replayState.nextAction = 0;

        // Move the agent back to the position of its first recorded action.
        // This helps the replay begin from the same location as the original run.
        agent.SetLocation(replayState.actions.front().Position);

        // Save this agent's replay information so Step() can process it later.
        mAgentReplays.push_back(replayState);
    }

    // Only mark the replay as running if at least one action exists.
    mRunning = HasRemainingActions();
}

void ReplayDriver::Stop() {
    // Stop the replay. Step() will return immediately while this is false.
    mRunning = false;
}

bool ReplayDriver::IsRunning() const {
    // Return whether the replay is currently active.
    return mRunning;
}

bool ReplayDriver::HasRemainingActions() const {
    // Check every agent replay to see if any still has actions left to process.
    for (const AgentReplayState& replayState : mAgentReplays) {
        if (replayState.nextAction < replayState.actions.size()) {
            return true;
        }
    }

    // If no agent has actions left, the replay is finished.
    return false;
}

void ReplayDriver::Step() {
    // Do not process replay steps if replay is stopped or no world is available.
    if (!mRunning || mWorld == nullptr) {
        return;
    }

    // Tracks whether this Step() call actually advanced at least one agent.
    bool processedAnyAction = false;

    // Process one action for each agent that still has replay actions remaining.
    for (AgentReplayState& replayState : mAgentReplays) {
        // Skip this agent if all of its recorded actions have already been replayed.
        if (replayState.nextAction >= replayState.actions.size()) {
            continue;
        }

        // Find the current agent in the world using the ID saved in its replay state.
        AgentBase* agent = mWorld->TryGetAgent(replayState.agentID);

        // If the agent no longer exists or is dead, skip this action.
        // The replay index still advances so the replay does not get stuck.
        if (agent == nullptr || !agent->IsAlive()) {
            ++replayState.nextAction;
            processedAnyAction = true;
            continue;
        }

        // Get the next recorded action for this agent.
        const Action& action = replayState.actions.at(replayState.nextAction);

        // Reset the agent to the action's starting position before applying the action.
        // This keeps the replay matched with the original recorded movement.
        agent->SetLocation(action.Position);

        // Convert the recorded action data into the action ID that this agent uses.
        const size_t action_id = GetReplayActionForAgent(*agent, action);

        // Run the action in the world and save the result back onto the agent.
        int result = mWorld->DoAction(*agent, action_id);
        agent->SetActionResult(result);

        // Move this agent to its next replay action for the next Step() call.
        ++replayState.nextAction;
        processedAnyAction = true;
    }

    // Stop replay if no action was processed, or if all actions are finished.
    if (!processedAnyAction || !HasRemainingActions()) {
        Stop();
    }
}

size_t ReplayDriver::GetReplayActionForAgent(const AgentBase& agent, const Action& action) const {
    // The recorded action stores the old position and the new position.
    // The difference between these positions tells us which movement happened.
    const WorldPosition& old_pos = action.Position;
    const WorldPosition& new_pos = action.NewPosition;

    // Handle interaction actions separately because they may not involve movement.
    if (action.ActionType == "interact") {
        // Prefer the named "interact" action if this agent supports it.
        if (agent.HasAction("interact")) {
            return agent.GetActionID("interact");
        }

        // Some agents may use "e" as the interaction key/action instead.
        if (agent.HasAction("e")) {
            return agent.GetActionID("e");
        }

        // If no interaction action exists, fall back to staying still.
        return agent.GetActionID("stay");
    }

    // Calculate the movement direction from the old position to the new position.
    const int dx = static_cast<int>(new_pos.X()) - static_cast<int>(old_pos.X());
    const int dy = static_cast<int>(new_pos.Y()) - static_cast<int>(old_pos.Y());

    // Match the recorded movement direction to the agent's action IDs.
    if (dx == 1 && dy == 0 && agent.HasAction("right")) {
        return agent.GetActionID("right");
    }

    if (dx == -1 && dy == 0 && agent.HasAction("left")) {
        return agent.GetActionID("left");
    }

    if (dx == 0 && dy == -1 && agent.HasAction("up")) {
        return agent.GetActionID("up");
    }

    if (dx == 0 && dy == 1 && agent.HasAction("down")) {
        return agent.GetActionID("down");
    }

    // If no movement action matches, use "stay" if the agent supports it.
    if (agent.HasAction("stay")) {
        return agent.GetActionID("stay");
    }

    // Final fallback if no matching action name exists.
    // This assumes action ID 0 is a safe/default action for the agent.
    return 0;
}

} // namespace cse498