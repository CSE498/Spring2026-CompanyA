/**
 * @file ActionLog.cpp
 * @brief Implementation of ActionLog, AgentActionLog, UserActionLog, and related helpers.
 */

#include "ActionLog.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace cse498 {

/**
 * @name ActionLog
 * @{
 */

/** Default constructor: empty log, time 0, sequence counter 0. */
ActionLog::ActionLog() : mNextSequenceNumber(0), mCurrentTime(0.0) {}

/**
 * @copydoc ActionLog::LogAction
 * @details Timestamps the entry with mCurrentTime and assigns the next sequence number.
 */
void ActionLog::LogAction(int entityId, const std::string& actionType, WorldPosition position,
                          WorldPosition newPosition) {
    Action action;
    action.EntityId = entityId;
    action.ActionType = actionType;
    action.Timestamp = mCurrentTime;
    action.Position = position;
    action.NewPosition = newPosition;
    action.SequenceNumber = mNextSequenceNumber++;
    mActions.push_back(action);
}

/**
 * @copydoc ActionLog::UpdateTime
 * @pre newTime must be non-negative (asserted in debug builds).
 */
void ActionLog::UpdateTime(double newTime) {
    assert(newTime >= 0.0 && "Time must be non-negative");
    mCurrentTime = newTime;
}

/** @copydoc ActionLog::GetActions */
const std::vector<Action>& ActionLog::GetActions() const { return mActions; }

/**
 * @copydoc ActionLog::GetActionRange
 * @pre startTime <= endTime (asserted in debug builds).
 */
std::vector<Action> ActionLog::GetActionRange(double startTime, double endTime) const {
    assert(startTime <= endTime && "startTime must be <= endTime");

    std::vector<Action> result;
    auto inRange = [startTime, endTime](const Action& a) { return a.Timestamp >= startTime && a.Timestamp <= endTime; };
    std::copy_if(mActions.begin(), mActions.end(), std::back_inserter(result), inRange);
    return result;
}

/** @copydoc ActionLog::GetEntityActions */
std::vector<Action> ActionLog::GetEntityActions(int entityId) const {
    std::vector<Action> result;
    std::copy_if(mActions.begin(), mActions.end(), std::back_inserter(result),
                 [entityId](const Action& a) { return a.EntityId == entityId; });
    return result;
}

/** @copydoc ActionLog::GetActionCount */
int ActionLog::GetActionCount() const { return static_cast<int>(mActions.size()); }

/**
 * @copydoc ActionLog::GetMostActiveEntity
 * @details Counts actions per entity and returns the ID with the highest count (first max if tied).
 */
std::optional<int> ActionLog::GetMostActiveEntity() const {
    if (mActions.empty()) {
        return std::nullopt;
    }

    std::unordered_map<int, int> counts;
    std::for_each(mActions.begin(), mActions.end(), [&counts](const Action& a) { ++counts[a.EntityId]; });

    auto maxEntry = std::max_element(counts.begin(), counts.end(),
                                     [](const auto& a, const auto& b) { return a.second < b.second; });

    return maxEntry->first;
}

/**
 * @copydoc ActionLog::GetActionCountInRange
 * @pre startTime <= endTime (asserted in debug builds).
 */
int ActionLog::GetActionCountInRange(double startTime, double endTime) const {
    assert(startTime <= endTime && "startTime must be <= endTime");

    return static_cast<int>(std::count_if(mActions.begin(), mActions.end(), [startTime, endTime](const Action& a) {
        return a.Timestamp >= startTime && a.Timestamp <= endTime;
    }));
}

/**
 * @copydoc ActionLog::Clear
 * @details Resets the sequence counter; does not change mCurrentTime.
 */
void ActionLog::Clear() {
    mActions.clear();
    mNextSequenceNumber = 0;
}

/** @} */

/**
 * @name AgentActionLog
 * @{
 */

/**
 * @copydoc AgentActionLog::GetStuckAgentRatio
 * @pre windowSize > 0 (asserted in debug builds).
 */
double AgentActionLog::GetStuckAgentRatio(int windowSize) const {
    assert(windowSize > 0 && "windowSize must be positive");

    std::unordered_set<int> agentIds;
    for (const auto& action: mActions) {
        agentIds.insert(action.EntityId);
    }

    if (agentIds.empty()) {
        return 0.0;
    }

    int stuckCount = std::count_if(agentIds.begin(), agentIds.end(),
                                   [this, windowSize](int id) { return cse498::IsEntityStuck(*this, id, windowSize); });

    return static_cast<double>(stuckCount) / static_cast<double>(agentIds.size());
}

/** @} */

/**
 * @name UserActionLog
 * @{
 */

/** @copydoc UserActionLog::GetLastAction */
std::optional<Action> UserActionLog::GetLastAction() const {
    if (mActions.empty()) {
        return std::nullopt;
    }
    return mActions.back();
}

/**
 * @copydoc UserActionLog::GetMostFrequentActionType
 * @details Counts ActionType strings and returns the label with the highest count (first max if tied).
 */
std::optional<std::string> UserActionLog::GetMostFrequentActionType() const {
    if (mActions.empty()) {
        return std::nullopt;
    }

    std::unordered_map<std::string, int> counts;
    std::for_each(mActions.begin(), mActions.end(), [&counts](const Action& a) { ++counts[a.ActionType]; });

    auto maxEntry = std::max_element(counts.begin(), counts.end(),
                                     [](const auto& a, const auto& b) { return a.second < b.second; });

    return maxEntry->first;
}

/** @} */

/**
 * @name Experimental helpers
 * @{
 */

/**
 * @copydoc IsEntityStuck
 * @pre windowSize > 0 (asserted in debug builds).
 * @details Requires at least windowSize actions for the entity; checks the last windowSize entries for zero displacement.
 */
bool IsEntityStuck(const ActionLog& log, int entityId, int windowSize) {
    assert(windowSize > 0 && "windowSize must be positive");

    std::vector<Action> entityActions = log.GetEntityActions(entityId);
    if (static_cast<int>(entityActions.size()) < windowSize) {
        return false;
    }

    auto begin = entityActions.end() - windowSize;
    return std::all_of(begin, entityActions.end(), [](const Action& a) {
        return a.Position.X() == a.NewPosition.X() && a.Position.Y() == a.NewPosition.Y();
    });
}

/**
 * @copydoc ExportToCsv
 * @details Writes a header row then one CSV line per action with positions split into X/Y columns.
 */
bool ExportToCsv(const ActionLog& log, const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    file << "SequenceNumber,EntityId,ActionType,Timestamp,X,Y,NewX,NewY\n";
    for (const auto& action: log.GetActions()) {
        file << action.SequenceNumber << "," << action.EntityId << "," << action.ActionType << "," << action.Timestamp
             << "," << action.Position.X() << "," << action.Position.Y() << "," << action.NewPosition.X() << ","
             << action.NewPosition.Y() << "\n";
    }
    return true;
}

/**
 * @copydoc Serialize
 * @details First line is the action count; each following line has sequence, entity, time, positions, then action type.
 */
std::string Serialize(const ActionLog& log) {
    std::ostringstream oss;
    oss << log.GetActionCount() << "\n";
    for (const auto& action: log.GetActions()) {
        oss << action.SequenceNumber << " " << action.EntityId << " " << action.Timestamp << " " << action.Position.X()
            << " " << action.Position.Y() << " " << action.NewPosition.X() << " " << action.NewPosition.Y() << " "
            << action.ActionType << "\n";
    }
    return oss.str();
}

/**
 * @copydoc Deserialize
 * @details Clears the log, reads the count, then repopulates via UpdateTime and LogAction for each entry.
 */
void Deserialize(ActionLog& log, const std::string& data) {
    log.Clear();

    std::istringstream iss(data);
    int count = 0;
    iss >> count;

    for (int i = 0; i < count; ++i) {
        Action action;
        double px, py, npx, npy;
        if (iss >> action.SequenceNumber >> action.EntityId >> action.Timestamp >> px >> py >> npx >> npy >>
            action.ActionType) {
            action.Position = WorldPosition{px, py};
            action.NewPosition = WorldPosition{npx, npy};
            log.UpdateTime(action.Timestamp);
            log.LogAction(action.EntityId, action.ActionType, action.Position, action.NewPosition);
        }
    }
}

/**
 * @copydoc ActionLog::IsEntityStuck
 * @details Delegates to cse498::IsEntityStuck.
 */
bool ActionLog::IsEntityStuck(int entityId, int windowSize) const {
    return cse498::IsEntityStuck(*this, entityId, windowSize);
}

/** @copydoc ActionLog::Serialize */
std::string ActionLog::Serialize() const { return cse498::Serialize(*this); }

/** @copydoc ActionLog::Deserialize */
void ActionLog::Deserialize(const std::string& data) { cse498::Deserialize(*this, data); }

/** @} */

} // namespace cse498
