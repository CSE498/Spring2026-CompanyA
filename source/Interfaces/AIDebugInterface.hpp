#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "../core/InterfaceBase.hpp"
#include "../core/WorldBase.hpp"

namespace cse498 {

  class AIDebugInterface : public InterfaceBase {
  private:
    bool step_mode = true;
    bool show_stats = true;
    size_t monitored_agent_id = 0;

    void DrawGrid(const WorldGrid & grid,
                  const std::vector<size_t> & item_ids,
                  const std::vector<size_t> & agent_ids) const;

    void PrintAgentSummary(const std::vector<size_t> & agent_ids) const;

    void PauseIfNeeded() const;

  public:
    AIDebugInterface(size_t id, const std::string & name, const WorldBase & world);
    ~AIDebugInterface() override = default;

    AIDebugInterface & SetStepMode(bool in) {
      step_mode = in;
      return *this;
    }

    AIDebugInterface & SetShowStats(bool in) {
      show_stats = in;
      return *this;
    }

    AIDebugInterface & SetMonitoredAgent(size_t id) {
      monitored_agent_id = id;
      return *this;
    }

    bool Initialize() override;
    [[nodiscard]] size_t SelectAction(const WorldGrid & grid) override;
  };

} // namespace cse498