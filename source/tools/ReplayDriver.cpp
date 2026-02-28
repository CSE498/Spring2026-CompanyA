#include "ReplayDriver.hpp"

// Placeholder for action verification logic
bool cse498::ReplayDriver::VerifyAction(const Action& action) { 
    if(true){
        m_lastReplayedActions.push_back(action);
        return true;
    }
    return false;
}

// Check if the ActionLog is set before replaying
bool cse498::ReplayDriver::IsActionLogSet() const { 
    if (!m_actionLog){
        printf("No ActionLog set for ReplayDriver.\n");
        return false;
    }
    return true;
}

 // Replays all actions in the ActionLog
void cse498::ReplayDriver::Replay() {
    if(IsActionLogSet()) {
        m_lastReplayedActions.clear(); // Clear previous replayed actions
        const std::vector<Action>& agentActions = m_actionLog->GetActions();  
        for(const Action& action : agentActions) {
            VerifyAction(action);
        }
    }
}

// Replays actions at intervals of 'step' (e.g., every 10th action)
void cse498::ReplayDriver::ReplayByStep(int step) { 
    if(IsActionLogSet() == true && step > 0) {
        m_lastReplayedActions.clear(); // Clear previous replayed actions
        for(int i = 0; i < m_actionLog->GetActionCount(); i += step) {
            Action action = m_actionLog->GetActions()[i];
            VerifyAction(action);
        }
    }
}

// Replays actions that occurred within a specific time range
void cse498::ReplayDriver::ReplayByTimeRange(double start_time, double end_time) { 
    if(IsActionLogSet() == true) {
        std::vector<Action> actionsInRange = m_actionLog->GetActionRange(start_time, end_time);
        m_lastReplayedActions.clear(); // Clear previous replayed actions
        for(Action action : actionsInRange) {
            VerifyAction(action);
        }
    }
}

// Replays actions performed by a specific agent
void cse498::ReplayDriver::ReplayByAgent(int agent_id) {
    if(IsActionLogSet() == true) {
        std::vector<Action> agentActions = m_actionLog->GetEntityActions(agent_id);
        m_lastReplayedActions.clear(); // Clear previous replayed actions
        for(Action action : agentActions) {
            VerifyAction(action);
        }
    }
}

// Replays actions of a specific type (e.g., "move", "attack")
void cse498::ReplayDriver::ReplayByActionType(const std::string& action_type) {
    if(IsActionLogSet() == true) {
        m_lastReplayedActions.clear(); // Clear previous replayed actions
        for(Action action : m_actionLog->GetActions()) {
            if(action.ActionType == action_type) {
                VerifyAction(action);
            }
        }
    }
}

// Saves the last replayed action command to a file in CSV format
bool cse498::ReplayDriver::SaveReplayToFile(const std::string& filename) {
    if(IsActionLogSet() == true) {
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            printf("Failed to open file: %s\n", filename.c_str());
            
            return false;
        }
        
        for(Action action : m_lastReplayedActions) {
            outFile << action.EntityId << "," 
                    << action.ActionType << "," 
                    << action.Timestamp << "," 
                    << action.X << "," 
                    << action.Y << "," 
                    << action.NewX << "," 
                    << action.NewY << "," 
                    << action.SequenceNumber << "\n";
        }
        outFile.close();
        return true;
    }
    return false;
}


