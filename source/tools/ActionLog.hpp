#ifndef ACTION_LOG_HPP
#define ACTION_LOG_HPP

#include <vector>
#include <string>

// Actions from an agent (to be printed later for ActionLog)
struct Action {
    int agent_id;
    std::string action_type;
    double timestamp; 
    double x;
    double y;
    double new_x;
    double new_y;
    int sequence_number;// Could be a string? Like "turn 1231231". Otherwise it could just be an int indicating the turn
};

// Tracks all the actions
class ActionLog {
private:
    std::vector<Action> actions; // Holds all the actions
    int next_sequence_number;
    double current_time;
    
public:
    ActionLog();
    
    void LogAction(int agent_id, const std::string& action_type, double x, double y, double new_x, double new_y); // Takes all params and logs a new action
    
    void UpdateTime(double new_time);
    
    // Getters
    const std::vector<Action>& GetActions() const;
    std::vector<Action> GetActionRange(double start_time, double end_time) const;
    std::vector<Action> GetAgentActions(int agent_id) const;
    int GetActionCount() const;
    
    void Clear(); // Deletes everything
};

#endif