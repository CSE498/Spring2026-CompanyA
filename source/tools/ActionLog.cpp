/**
 * @file ActionLog.cpp
 * @brief Implementation of ActionLog, AgentActionLog, and UserActionLog.
 */

#include "ActionLog.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace cse498 {

// ActionLog

ActionLog::ActionLog()
    : NextSequenceNumber(0), CurrentTime(0.0) {}

void ActionLog::LogAction(int entityId, const std::string& actionType,
                          double x, double y, double newX, double newY) {
    Action action;
    action.EntityId       = entityId;
    action.ActionType     = actionType;
    action.Timestamp      = CurrentTime;
    action.X              = x;
    action.Y              = y;
    action.NewX           = newX;
    action.NewY           = newY;
    action.SequenceNumber = NextSequenceNumber++;

    Actions.push_back(action);
}

void ActionLog::UpdateTime(double newTime) {
    assert(newTime >= 0.0 && "Time must be non-negative");
    CurrentTime = newTime;
}

const std::vector<Action>& ActionLog::GetActions() const {
    return Actions;
}

std::vector<Action> ActionLog::GetActionRange(double startTime, double endTime) const {
    assert(startTime <= endTime && "startTime must be <= endTime");

    std::vector<Action> result;
    for (const auto& action : Actions) {
        if (action.Timestamp >= startTime && action.Timestamp <= endTime) {
            result.push_back(action);
        }
    }
    return result;
}

std::vector<Action> ActionLog::GetEntityActions(int entityId) const {
    std::vector<Action> result;
    for (const auto& action : Actions) {
        if (action.EntityId == entityId) {
            result.push_back(action);
        }
    }
    return result;
}

int ActionLog::GetActionCount() const {
    return static_cast<int>(Actions.size());
}

void ActionLog::Clear() {
    Actions.clear();
    NextSequenceNumber = 0;
}

// AgentActionLog

double AgentActionLog::GetStuckAgentRatio(int windowSize) const {
    assert(windowSize > 0 && "windowSize must be positive");

    std::unordered_set<int> agentIds;
    for (const auto& action : Actions) {
        agentIds.insert(action.EntityId);
    }

    if (agentIds.empty()) {
        return 0.0;
    }

    int stuckCount = 0;
    for (int id : agentIds) {
        if (IsEntityStuck(*this, id, windowSize)) {
            ++stuckCount;
        }
    }

    return static_cast<double>(stuckCount) / static_cast<double>(agentIds.size());
}

// UserActionLog

std::optional<Action> UserActionLog::GetLastAction() const {
    if (Actions.empty()) {
        return std::nullopt;
    }
    return Actions.back();
}

std::optional<std::string> UserActionLog::GetMostFrequentActionType() const {
    if (Actions.empty()) {
        return std::nullopt;
    }

    std::unordered_map<std::string, int> counts;
    for (const auto& action : Actions) {
        ++counts[action.ActionType];
    }

    auto maxEntry = std::max_element(counts.begin(), counts.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    return maxEntry->first;
}

// Experimental

bool IsEntityStuck(const ActionLog& log, int entityId, int windowSize) {
    assert(windowSize > 0 && "windowSize must be positive");

    std::vector<Action> entityActions = log.GetEntityActions(entityId);
    if (static_cast<int>(entityActions.size()) < windowSize) {
        return false;
    }

    auto begin = entityActions.end() - windowSize;
    for (auto it = begin; it != entityActions.end(); ++it) {
        if (it->X != it->NewX || it->Y != it->NewY) {
            return false;
        }
    }
    return true;
}

bool ExportToCsv(const ActionLog& log, const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    file << "SequenceNumber,EntityId,ActionType,Timestamp,X,Y,NewX,NewY\n";
    for (const auto& action : log.GetActions()) {
        file << action.SequenceNumber << ","
             << action.EntityId       << ","
             << action.ActionType     << ","
             << action.Timestamp      << ","
             << action.X              << ","
             << action.Y              << ","
             << action.NewX           << ","
             << action.NewY           << "\n";
    }
    return true;
}

std::string Serialize(const ActionLog& log) {
    std::ostringstream oss;
    oss << log.GetActionCount() << "\n";
    for (const auto& action : log.GetActions()) {
        oss << action.SequenceNumber << " "
            << action.EntityId       << " "
            << action.Timestamp      << " "
            << action.X              << " "
            << action.Y              << " "
            << action.NewX           << " "
            << action.NewY           << " "
            << action.ActionType     << "\n";
    }
    return oss.str();
}

void Deserialize(ActionLog& log, const std::string& data) {
    log.Clear();

    std::istringstream iss(data);
    int count = 0;
    iss >> count;

    for (int i = 0; i < count; ++i) {
        Action action;
        if (iss >> action.SequenceNumber
                >> action.EntityId
                >> action.Timestamp
                >> action.X
                >> action.Y
                >> action.NewX
                >> action.NewY
                >> action.ActionType) {
            log.UpdateTime(action.Timestamp);
            log.LogAction(action.EntityId, action.ActionType,
                          action.X, action.Y, action.NewX, action.NewY);
        }
    }
}

}