/**
 * @file FarmingAgent.hpp
 * @brief Minimal FarmingAgent (trading NPC stub).
 */
#pragma once


#include "MerchantAgent.hpp"

namespace cse498
{

    /** Basic NPC agent used for simple trading interactions. */
    class FarmingAgent : public MerchantAgent
    {
    public:
        FarmingAgent(std::size_t id, const std::string& name, WorldBase& world);
        [[nodiscard]] std::string GetTradeGreeting() const override;
    };
}

