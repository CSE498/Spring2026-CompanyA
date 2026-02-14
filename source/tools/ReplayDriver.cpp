#include "ReplayDriver.h"

// Placeholder for action verification logic
bool ReplayDriver::verifyAction(const Action& action) { 
    if(true){
        lastReplayedActions.push_back(action);
        return true;
    }
    return false;
}

// Check if the ActionLog is set before replaying
bool ReplayDriver::IsActionLogSet() { 
    if(m_actionLog == nullptr) {
        printf("No ActionLog set for ReplayDriver.\n");
        return false;
    }
    return true;
}

 // Replays all actions in the ActionLog
void ReplayDriver::Replay() {
    if(IsActionLogSet() == true) {
        lastReplayedActions.clear(); // Clear previous replayed actions
        std::vector<Action> agentActions = m_actionLog->GetActions();  
        for(Action action : agentActions) {
            verifyAction(action);
        }
    }
}

// Replays actions at intervals of 'step' (e.g., every 10th action)
void ReplayDriver::ReplayByStep(int step) { 
    if(IsActionLogSet() == true && step > 0) {
        lastReplayedActions.clear(); // Clear previous replayed actions
        for(int i = 0; i < m_actionLog->GetActionCount(); i += step) {
            Action action = m_actionLog->GetActions()[i];
            verifyAction(action);
        }
    }
}

// Replays actions that occurred within a specific time range
void ReplayDriver::ReplayByTimeRange(double start_time, double end_time) { 
    if(IsActionLogSet() == true) {
        std::vector<Action> actionsInRange = m_actionLog->GetActionRange(start_time, end_time);
        lastReplayedActions.clear(); // Clear previous replayed actions
        for(Action action : actionsInRange) {
            verifyAction(action);
        }
    }
}

// Replays actions performed by a specific agent
void ReplayDriver::ReplayByAgent(int agent_id) {
    if(IsActionLogSet() == true) {
        std::vector<Action> agentActions = m_actionLog->GetAgentActions(agent_id);
        lastReplayedActions.clear(); // Clear previous replayed actions
        for(Action action : agentActions) {
            verifyAction(action);
        }
    }
}

// Replays actions of a specific type (e.g., "move", "attack")
void ReplayDriver::ReplayByActionType(const std::string& action_type) {
    if(IsActionLogSet() == true) {
        lastReplayedActions.clear(); // Clear previous replayed actions
        for(Action action : m_actionLog->GetActions()) {
            if(action.action_type == action_type) {
                verifyAction(action);
            }
        }
    }
}

// Saves the last replayed action command to a file in CSV format
bool ReplayDriver::SaveReplayToFile(const std::string& filename) {
    if(IsActionLogSet() == true) {
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            printf("Failed to open file: %s\n", filename.c_str());
            
            return false;
        }
        
        for(Action action : lastReplayedActions) {
            outFile << action.agent_id << "," 
                    << action.action_type << "," 
                    << action.timestamp << "," 
                    << action.x << "," 
                    << action.y << "," 
                    << action.new_x << "," 
                    << action.new_y << "," 
                    << action.sequence_number << "\n";
        }
        outFile.close();
        return true;
    }
    return false;
}


