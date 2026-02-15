#include "ActionLog.hpp"
#include <fstream>

ActionLog::ActionLog() 
    : next_sequence_number(0), current_time(0.0) {
}

void ActionLog::LogAction(int agent_id, const std::string& action_type, double x, double y, double new_x, double new_y) {
    Action action;
    action.agent_id = agent_id;
    action.action_type = action_type;
    action.timestamp = current_time;
    action.x = x;
    action.y = y;
    action.new_x = new_x;
    action.new_y = new_y;
    action.sequence_number = next_sequence_number++;
    
    actions.push_back(action);
}

void ActionLog::UpdateTime(double new_time) {
    current_time = new_time;
}

const std::vector<Action>& ActionLog::GetActions() const {
    return actions;
}

std::vector<Action> ActionLog::GetActionRange(double start_time, double end_time) const {
    std::vector<Action> result;
    for (const auto& action : actions) {
        if (action.timestamp >= start_time && action.timestamp <= end_time) {
            result.push_back(action);
        }
    }
    return result;
}

std::vector<Action> ActionLog::GetAgentActions(int agent_id) const {
    std::vector<Action> result;
    for (const auto& action : actions) {
        if (action.agent_id == agent_id) {
            result.push_back(action);
        }
    }
    return result;
}

int ActionLog::GetActionCount() const {
    return actions.size();
}

void ActionLog::Clear() {
    actions.clear();
    next_sequence_number = 0;
}