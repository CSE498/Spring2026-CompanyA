/**
* @file FarmingAgent.cpp
 * @brief Minimal FarmingAgent implementation (trading NPC stub).
 */

#include "FarmingAgent.hpp"

namespace cse498
{

    FarmingAgent::FarmingAgent(std::size_t id, const std::string& name, WorldBase& world)
        : MerchantAgent(id, name, world)
    {
        // Shop setup in world
    }

    std::string FarmingAgent::GetTradeGreeting() const
    {
        return "Fresh crops and seeds today.";
    }


}