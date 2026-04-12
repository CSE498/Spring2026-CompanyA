/**
 *
 * @note Status: PROPOSAL
 *
 * The goal of this class is to act as a container to store all the DataLogs and
 * ActionLogs for the project. This class will be called by the game to log data
 * and actions which will be used by the StatsTracker class.
 **/

#pragma once
#include <string>
#include "../tools/DataLog.hpp"
#include "../tools/ActionLog.hpp"

namespace cse498 {

class AnalyticsManager {
private:

  // Initial DataLogs for keeping track of data(Initial placeholders Logs for
  // now, can be changed based on what we want to track)
  DataLog mHealthLog;
  DataLog mEnemiesTrackedLog;
  DataLog mDamageDealtLog;

  

  // ActionLog for tracking entity actions
  ActionLog mActionLog;

public:
  /*
  Default constructor for the AnalyticsManager
  */
  AnalyticsManager() = default;

  /*
  Resets the health log
  */
  void ResetHealthLog();

  /*
  Resets the enemies tracked log
  */
  void ResetEnemiesTrackedLog();

  /*
  Resets the damage dealt log
  */
  void ResetDamageDealtLog();

  /**
  * Resets the action log
  */
  void ResetActionLog();

  /**
   * Resets all logs
   */
  void Reset();

  // Temporary DataLogs for the game(can be changed based on what we want to
  // track)
  /*
  Adds a new health value to the health log
  */
  void LogHealth(double health);

  /*
  Adds a new enemy count value to the enemies tracked log
  */
  void LogEnemiesTracked(double count_enemies);

  /*
  Adds a new damage dealt value to the damage dealt log
  */
  void LogDamageDealt(double damage);

  /*
  Returns a const reference to the health log
  */
  [[nodiscard]] const DataLog &GetHealthLog() const noexcept;

  /*
  Returns a const reference to the enemies tracked log
  */
  [[nodiscard]] const DataLog &GetEnemiesTrackedLog() const noexcept;

  /*
  Returns a const reference to the damage dealt log
  */
  [[nodiscard]] const DataLog &GetDamageDealtLog() const noexcept;

  /*
  Logs an action performed by an entity into the action log
  */
  void LogAction(int entityId, const std::string& actionType,
                 WorldPosition position, WorldPosition newPosition);

  /*
  Advances the action log's simulation clock
  */
  void UpdateActionTime(double newTime);

  /*
  Returns a const reference to the action log
  */
  [[nodiscard]] const ActionLog& GetActionLog() const noexcept;
};

} // namespace cse498