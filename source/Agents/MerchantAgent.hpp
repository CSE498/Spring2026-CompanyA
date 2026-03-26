/**
 * @file MerchantAgent.hpp
 * @author Ty Maksimowski
 *
 * Basic merchant NPC that can trade with the player.
 */

#pragma once
#include <cstddef>
#include <string>
#include <vector>

#include "../core/AgentBase.hpp"
#include "TradeSystem/TradeSystem.hpp"
#include "PlayerAgent.hpp"

namespace cse498
{
    class MerchantAgent : public AgentBase
    {
    private:
        bool mAvailableForTrade = true;
        std::vector<TradeOffer> mOffers;
        std::size_t mNextItemId = 10000;
        std::size_t mGold = 0;

    protected:
        void ClearOffers();
        void AddOffer(const TradeOffer& offer);

        [[nodiscard]] TradeOffer* FindOfferMutable(const std::string& itemName);
        TradeOffer& EnsureLimitedOffer(const std::string& itemName, std::size_t buyPrice,
                                       std::size_t sellPrice, int itemValue);

    public:
        MerchantAgent(std::size_t id, const std::string& name, WorldBase& world);

        [[nodiscard]] std::size_t SelectAction(const WorldGrid& grid) override;

        [[nodiscard]] bool IsAvailableForTrade() const;
        void SetAvailableForTrade(bool available);

        [[nodiscard]] const std::vector<TradeOffer>& GetOffers() const;
        [[nodiscard]] const TradeOffer* FindOffer(const std::string& itemName) const;

        [[nodiscard]] std::size_t GetGold() const { return mGold; }
        void AddGold(std::size_t amount) { mGold += amount; }
        bool SpendGold(std::size_t amount);

        TradeResult BuyFromMerchant(PlayerAgent& player, const std::string& itemName, std::size_t quantity = 1);
        TradeResult SellToMerchant(PlayerAgent& player, const std::string& itemName, std::size_t quantity = 1);

        [[nodiscard]] virtual std::string GetTradeGreeting() const;
        [[nodiscard]] virtual std::string GetTradeClosedMessage() const;
    };
} // cse498
