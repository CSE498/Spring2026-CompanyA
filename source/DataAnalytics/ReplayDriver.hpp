#ifndef REPLAYDRIVER_HPP
#define REPLAYDRIVER_HPP

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "../tools/ActionLog.hpp"
#include "../Interfaces/TrashInterface.hpp"
#include "../Worlds/MazeWorld.hpp"

namespace cse498 {

class ReplayDriver {
private:
  std::shared_ptr<const ActionLog> m_actionLog;
  int m_delay;
  void ReplayAction(const cse498::Action &action, cse498::TrashInterface &agent, MazeWorld &world);
  
public:
/**
 * Creates the Replay Driver class
 * ActionLog of all action
 * int Delay (how long each second of replay should take)
 */
  ReplayDriver(std::shared_ptr<const ActionLog> log, int delay = 500)
      : m_actionLog(std::move(log)), m_delay(delay) {}

  /**
   * Set a different action Log if wanted
   * ActionLog log to be added
   */
  void setActionLog(std::shared_ptr<const ActionLog> log) {
    m_actionLog = std::move(log);
  }
  /**
   * Replays the action log from a certain time
   * double start_time
   * doulbe end_time
   */
  void ReplayByTimeRange(double start_time, double end_time);
  /**
   * Relpays the full game
   */
  void ReplayFullGame();
};

} // namespace cse498
#endif
