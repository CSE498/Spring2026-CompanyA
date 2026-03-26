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
        ClearOffers();

        /**
         * -seeds are unlimited
         * -Wheat and carrots are limited
         */
        AddOffer({"wheat", 3, 1, 1, TradeStockMode::Limited, 30});
        AddOffer({"seeds", 2, 1, 1, TradeStockMode::Unlimited, 0});
        AddOffer({"carrot", 4, 2, 1, TradeStockMode::Limited, 18});
    }

    std::string FarmingAgent::GetTradeGreeting() const
    {
        return "Fresh crops and seeds today.";
    }


}