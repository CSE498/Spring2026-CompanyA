#include "../../source/tools/ActionLog.hpp"

#include <cassert>
#include <iostream>

int main1() {
    ActionLog log;

    // inital state
    assert(log.GetActionCount() == 0);

    // action logging
    log.UpdateTime(1.0);
    log.LogAction(1, "move", 0, 0, 5, 5);

    log.UpdateTime(2.0);
    log.LogAction(2, "move", 1, 1, 6, 6);

    assert(log.GetActionCount() == 2);

    // tests getting actions
    const auto& actions = log.GetActions();
    assert(actions[0].agent_id == 1);
    assert(actions[1].agent_id == 2);
    assert(actions[0].sequence_number == 0);
    assert(actions[1].sequence_number == 1);

    // getagentactions
    auto agent1Actions = log.GetAgentActions(1);
    assert(agent1Actions.size() == 1);
    assert(agent1Actions[0].agent_id == 1);

    // getactionrange
    auto range = log.GetActionRange(1.5, 3.0);
    assert(range.size() == 1);
    assert(range[0].agent_id == 2);

    // clearing
    log.Clear();
    assert(log.GetActionCount() == 0);

    std::cout << "tests passed";
    return 0;
}