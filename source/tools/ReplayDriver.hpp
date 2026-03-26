#ifndef REPLAYDRIVER_HPP
#define REPLAYDRIVER_HPP

#include "ActionLog.hpp"

#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace cse498 {

class ReplayDriver {
private:
  std::shared_ptr<const ActionLog> m_actionLog;
  bool VerifyAction(const Action &action);
  std::vector<Action>
      m_lastReplayedActions; // Store the last set of actions that were replayed
                             // for potential verification or analysis
public:
  ReplayDriver() = default;
  explicit ReplayDriver(std::shared_ptr<const ActionLog> log)
      : m_actionLog(std::move(log)) {}

  void setActionLog(std::shared_ptr<const ActionLog> log) {
    m_actionLog = std::move(log);
  }
  bool IsActionLogSet() const;

  void Replay();
  void ReplayByStep(int step);
  void ReplayByTimeRange(double start_time, double end_time);
  void ReplayByAgent(int agent_id);
  void ReplayByActionType(const std::string &action_type);

  bool SaveReplayToFile(const std::string &filename);
};

} // namespace cse498
#endif
