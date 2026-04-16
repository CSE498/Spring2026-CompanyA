/**
 * @file ActionLog.hpp
 * Tracks and stores actions performed by agents and users.
 * Provides base ActionLog class and two subclasses: AgentActionLog for tracking
 * enemy/NPC agent actions, and UserActionLog for tracking player/user actions.
 */

#ifndef ACTION_LOG_HPP
#define ACTION_LOG_HPP

#include <cassert>
#include <optional>
#include <string>
#include <vector>
#include "../core/WorldPosition.hpp"

namespace cse498 {

/// Represents a single recorded action at a point in time.
struct Action {
    int EntityId; // ID of the agent
    std::string ActionType; // Descriptive label
    double Timestamp; // Simulation time when the action occurred
    WorldPosition Position; // Position before action
    WorldPosition NewPosition; // Position after action
    int SequenceNumber; // Action index
};

/**
 * Base class for recording and querying a sequence of actions.
 * Maintains an ordered log of Action entries, each stamped with the current
 * simulation time.
 */
class ActionLog {
public:
    ActionLog();
    virtual ~ActionLog() = default;

    /**
     * Records a new action.
     * @param entityId     ID of the entity performing the action
     * @param actionType   Human-readable action label
     * @param x            Entity's position before the action
     * @param y            Entity's position before the action
     * @param newX         Entity's position after the action
     * @param newY         Entity's position after the action
     */
    void LogAction(int entityId, const std::string& actionType, WorldPosition position, WorldPosition newPosition);

    /**
     * Advances the simulation clock used to timestamp future actions.
     * @param newTime  New current time; must be >= 0
     */
    void UpdateTime(double newTime);

    /// Returns all recorded actions in order.
    const std::vector<Action>& GetActions() const;

    /// Returns actions whose timestamps fall within [startTime, endTime].
    std::vector<Action> GetActionRange(double startTime, double endTime) const;

    /// Returns all actions performed by the given entity.
    std::vector<Action> GetEntityActions(int entityId) const;

    /// Returns the total number of recorded actions.
    int GetActionCount() const;

    /// Returns the ID of the entity with the most logged actions, or nullopt if
    /// empty.
    std::optional<int> GetMostActiveEntity() const;

    /// Returns the number of actions whose timestamps fall within [startTime,
    /// endTime].
    int GetActionCountInRange(double startTime, double endTime) const;

    /// Clears all actions and resets sequence counter (CurrentTime is preserved).
    void Clear();

    /// add comments for params later gotta fix nahum issues first
    bool IsEntityStuck(int entityId, int windowSize = 5) const;
    std::string Serialize() const;
    void Deserialize(const std::string& data);

protected:
    std::vector<Action> mActions;
    int mNextSequenceNumber;
    double mCurrentTime;
};

/**
 * ActionLog specialized for enemy/NPC agent actions.
 * Extends ActionLog with agent-specific analytics.
 */
class AgentActionLog : public ActionLog {
public:
    AgentActionLog() = default;

    /**
     * Returns the proportion of distinct agents that appear stuck.
     * @param windowSize  Passed to IsEntityStuck for each agent (must be > 0)
     * @return Value in [0.0, 1.0]; returns 0.0 if no agents have been logged
     */
    double GetStuckAgentRatio(int windowSize = 5) const;
};

/**
 * ActionLog specialized for player actions.
 */
class UserActionLog : public ActionLog {
public:
    UserActionLog() = default;

    /**
     * Returns the most recently logged user action, if any.
     */
    std::optional<Action> GetLastAction() const;

    /**
     * Returns the action type the user has performed most often.
     * @return Most common ActionType string, or std::nullopt if the log is empty
     */
    std::optional<std::string> GetMostFrequentActionType() const;
};

// Experimental

/**
 * Checks whether an entity appears stuck over its last actions.
 * @param entityId    Entity to inspect
 * @param windowSize  Number of recent actions to examine (must be > 0)
 * @return true if all recent actions show zero displacement
 */
bool IsEntityStuck(const ActionLog& log, int entityId, int windowSize = 5);

/**
 * Exports a log to a CSV file for external analysis.
 * @param log       Log to export
 * @param filePath  Destination file path
 * @return true on success, false if the file could not be opened
 */
bool ExportToCsv(const ActionLog& log, const std::string& filePath);

/**
 * Serializes a log's state to a string for save/resume.
 * @return Newline-delimited representation of all actions
 */
std::string Serialize(const ActionLog& log);

/**
 * Restores log state previously produced by Serialize().
 * @param log   Log to restore into; replaces any existing content
 * @param data  Serialized string
 */
void Deserialize(ActionLog& log, const std::string& data);

} // namespace cse498

#endif // ACTION_LOG_HPP
