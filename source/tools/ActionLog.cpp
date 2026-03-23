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
#include <iterator>

namespace cse498 {

// ActionLog

ActionLog::ActionLog()
    : NextSequenceNumber(0), CurrentTime(0.0) {}

void ActionLog::LogAction(int entityId, const std::string& actionType,
                          WorldPosition position, WorldPosition newPosition) {
    Action action;
    action.EntityId       = entityId;
    action.ActionType     = actionType;
    action.Timestamp      = CurrentTime;
    action.Position       = position;
    action.NewPosition    = newPosition;
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
    auto inRange = [startTime, endTime](const Action& a) {
        return a.Timestamp >= startTime && a.Timestamp <= endTime;
    };
    std::copy_if(Actions.begin(), Actions.end(), std::back_inserter(result), inRange);
    return result;
}

std::vector<Action> ActionLog::GetEntityActions(int entityId) const {
    std::vector<Action> result;
    std::copy_if(Actions.begin(), Actions.end(), std::back_inserter(result), [entityId](const Action& a) { return a.EntityId == entityId; });
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

    int stuckCount = std::count_if(agentIds.begin(), agentIds.end(),
    [this, windowSize](int id) {
        return cse498::IsEntityStuck(*this, id, windowSize);
    });

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
    std::for_each(Actions.begin(), Actions.end(), [&counts](const Action& a) { ++counts[a.ActionType]; });

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
    return std::all_of(begin, entityActions.end(), [](const Action& a) {
        return a.Position.X() == a.NewPosition.X() &&
            a.Position.Y() == a.NewPosition.Y();
    });
}

bool ExportToCsv(const ActionLog& log, const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    file << "SequenceNumber,EntityId,ActionType,Timestamp,X,Y,NewX,NewY\n";
    for (const auto& action : log.GetActions()) {
        file << action.SequenceNumber  << ","
             << action.EntityId        << ","
             << action.ActionType      << ","
             << action.Timestamp       << ","
             << action.Position.X()    << ","
             << action.Position.Y()    << ","
             << action.NewPosition.X() << ","
             << action.NewPosition.Y() << "\n";
    }
    return true;
}

std::string Serialize(const ActionLog& log) {
    std::ostringstream oss;
    oss << log.GetActionCount() << "\n";
    for (const auto& action : log.GetActions()) {
        oss << action.SequenceNumber  << " "
            << action.EntityId        << " "
            << action.Timestamp       << " "
            << action.Position.X()    << " "
            << action.Position.Y()    << " "
            << action.NewPosition.X() << " "
            << action.NewPosition.Y() << " "
            << action.ActionType      << "\n";
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
        double px, py, npx, npy;
        if (iss >> action.SequenceNumber
        >> action.EntityId
        >> action.Timestamp
        >> px >> py >> npx >> npy
        >> action.ActionType) {
            action.Position    = WorldPosition{px, py};
            action.NewPosition = WorldPosition{npx, npy};
            log.UpdateTime(action.Timestamp);
            log.LogAction(action.EntityId, action.ActionType, action.Position, action.NewPosition);
        }
    }
}

bool ActionLog::IsEntityStuck(int entityId, int windowSize) const {
    return cse498::IsEntityStuck(*this, entityId, windowSize);
}

std::string ActionLog::Serialize() const {
    return cse498::Serialize(*this);
}

void ActionLog::Deserialize(const std::string& data) {
    cse498::Deserialize(*this, data);
}

}