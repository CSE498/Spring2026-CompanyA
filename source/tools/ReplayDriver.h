#ifndef REPLAYDRIVER_H
#define REPLAYDRIVER_H

#include "ActionLog.hpp"

#include <string>
#include <vector>  
#include <fstream>


class ReplayDriver {
private:
    const ActionLog* m_actionLog = nullptr;
    bool verifyAction(const Action& action);
    std::vector<Action> lastReplayedActions; // Store the last set of actions that were replayed for potential verification or analysis
public:
    ReplayDriver() = default;
    explicit ReplayDriver(const ActionLog& log) : m_actionLog(&log) {}

    void setActionLog(const ActionLog& log) { m_actionLog = &log; }
    bool IsActionLogSet();

    void Replay();
    void ReplayByStep(int step);
    void ReplayByTimeRange(double start_time, double end_time);
    void ReplayByAgent(int agent_id);
    void ReplayByActionType(const std::string& action_type);

    bool SaveReplayToFile(const std::string& filename);
};

#endif
