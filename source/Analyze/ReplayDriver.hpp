#ifndef REPLAYDRIVER_HPP
#define REPLAYDRIVER_HPP

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "../Interfaces/TrashInterface.hpp"
#include "../Worlds/MazeWorld.hpp"
#include "../tools/ActionLog.hpp"

namespace cse498 {

class ReplayDriver {
private:
    std::shared_ptr<const ActionLog> m_actionLog; // action log of all actions to be replayed
    int m_delay; // delay for how long each move in the replay should take (in milliseconds)
    void ReplayAction(const cse498::Action& action, cse498::TrashInterface& agent,
                      MazeWorld& world); // helper function to replay a single action
public:
    /**
     * Constructs a ReplayDriver with an action log and optional replay delay.
     *
     * @param log Shared pointer to the ActionLog containing actions to replay.
     * @param delay Delay between replayed actions in milliseconds.
     *              Defaults to 500.
     */
    ReplayDriver(std::shared_ptr<const ActionLog> log, int delay = 500) : m_actionLog(std::move(log)), m_delay(delay) {}

    /**
     * Replaces the current action log with a new one.
     *
     * @param log Shared pointer to the new ActionLog to use.
     */
    void SetActionLog(std::shared_ptr<const ActionLog> log) { m_actionLog = std::move(log); }

    /**
     * Replays actions whose timestamps fall within the given time range.
     *
     * @param start_time The beginning of the time range.
     * @param end_time The end of the time range.
     */
    void ReplayByTimeRange(double start_time, double end_time);

    /**
     * Replays the entire action log from start to finish.
     */
    void ReplayFullGame();
};

} // namespace cse498
#endif
