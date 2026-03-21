/**
* @file FarmingAgent.cpp
 * @brief Minimal FarmingAgent implementation (trading NPC stub).
 */

#include "FarmingAgent.h"

namespace cse498
{

    FarmingAgent::FarmingAgent(size_t id, const std::string & name, WorldBase & world)
      : AgentBase(id, name, world) {}

    size_t FarmingAgent::SelectAction([[maybe_unused]] const WorldGrid & grid)
    {
        return 0;
    }

    bool FarmingAgent::IsAvailableForTrade() const
    {
        return mAvailableForTrade;
    }

    void FarmingAgent::SetAvailableForTrade(bool available)
    {
        mAvailableForTrade = available;
    }

}