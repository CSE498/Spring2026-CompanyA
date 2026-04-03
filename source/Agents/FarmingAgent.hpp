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
        /**
         * Constructs a farming merchant with crop focused offers
         *
         * @param id Unique agent id
         * @param name NPC display name
         * @param world Owning world
         */
        FarmingAgent(std::size_t id, const std::string& name, WorldBase& world);

        /**
         * @return Farmer specific trade greeting
         */
        [[nodiscard]] std::string GetTradeGreeting() const override;
    };
}

