/**
 * @file MerchantAgent.cpp
 * @author Ty Maksimowski
 */

#include "MerchantAgent.hpp"

#include <algorithm>

namespace cse498
{
    MerchantAgent::MerchantAgent(std::size_t id, const std::string& name, WorldBase& world)
        : AgentBase(id, name, world)
    {
        /**
         * - apple stock is unlimited
         * - bread and potion stocks are limited
         */
        AddOffer({"apple", 4, 2, 1, TradeStockMode::Unlimited, 0});
        AddOffer({"bread", 6, 3, 1, TradeStockMode::Limited, 18});
        AddOffer({"potion", 10, 5, 1, TradeStockMode::Limited, 10});

        AddGold(200);
    }

    std::size_t MerchantAgent::SelectAction([[maybe_unused]] const WorldGrid& grid)
    {
        return 0;
    }

    bool MerchantAgent::IsAvailableForTrade() const
    {
        return mAvailableForTrade;
    }

    void MerchantAgent::SetAvailableForTrade(bool available)
    {
        mAvailableForTrade = available;
    }

    const std::vector<TradeOffer>& MerchantAgent::GetOffers() const
    {
        return mOffers;
    }

    const TradeOffer* MerchantAgent::FindOffer(const std::string& itemName) const
    {
        const auto it = std::ranges::find_if(mOffers,
                                             [&itemName](const TradeOffer& offer)
                                             {
                                                 return offer.mItemName == itemName;
                                             });

        return (it == mOffers.end()) ? nullptr : &(*it);
    }

    bool MerchantAgent::SpendGold(const std::size_t amount)
    {
        if (amount > mGold)
        {
            return false;
        }

        mGold -= amount;
        return true;
    }

    TradeResult MerchantAgent::BuyFromMerchant(PlayerAgent& player, const std::string& itemName, std::size_t quantity)
    {
        if (!mAvailableForTrade)
        {
            return {
                TradeStatus::MerchantClosed,
                GetTradeClosedMessage(),
                itemName,
                quantity,
                0
            };
        }

        TradeOffer* offer = FindOfferMutable(itemName);
        if (offer == nullptr)
        {
            return {
                TradeStatus::UnknownItem,
                "I do not sell that item.",
                itemName,
                quantity,
                0
            };
        }

        return TradeSystem::BuyItem(
            GetWorld(),
            mNextItemId,
            player,
            *this,
            *offer,
            quantity
        );
    }

    TradeResult MerchantAgent::SellToMerchant(PlayerAgent& player, const std::string& itemName, std::size_t quantity)
    {
        if (!mAvailableForTrade)
        {
            return {
                TradeStatus::MerchantClosed,
                GetTradeClosedMessage(),
                itemName,
                quantity,
                0
            };
        }

        if (quantity == 0)
        {
            return {
                TradeStatus::InvalidQuantity,
                "Quantity must be at least 1.",
                itemName,
                0,
                0
            };
        }

        const Item* item = player.GetInventory().FindFirstItem(itemName);
        if (item == nullptr)
        {
            return {
                TradeStatus::PlayerOutOfStock,
                "You do not have that item.",
                itemName,
                quantity,
                0
            };
        }

        const int itemValue = item->GetGold();
        const std::size_t buyPrice = static_cast<std::size_t>(std::max(1, itemValue));
        const std::size_t sellPrice = std::max<std::size_t>(1, buyPrice / 2);

        TradeOffer& offer = EnsureLimitedOffer(item->GetName(), buyPrice, sellPrice, itemValue);

        return TradeSystem::SellItem(
            GetWorld(),
            mNextItemId,
            player,
            *this,
            offer,
            quantity
        );
    }

    TradeOffer* MerchantAgent::FindOfferMutable(const std::string& itemName)
    {
        const auto it = std::ranges::find_if(mOffers,
            [&itemName](const TradeOffer& offer){ return offer.mItemName == itemName; });

        return (it == mOffers.end()) ? nullptr : &(*it);
    }

    TradeOffer& MerchantAgent::EnsureLimitedOffer(const std::string& itemName, std::size_t buyPrice,
                                                  std::size_t sellPrice, int itemValue)
    {
        if (TradeOffer* existing = FindOfferMutable(itemName); existing != nullptr)
        {
            return *existing;
        }

        /**
         * If the merchant does not already sell this item, create a new
         * limited-stock listing so the player can buy it back later.
         */
        mOffers.push_back(TradeOffer{
            .mItemName = itemName,
            .mBuyPrice = buyPrice,
            .mSellPrice = sellPrice,
            .mItemValue = itemValue,
            .mStockMode = TradeStockMode::Limited,
            .mStock = 0
        });

        return mOffers.back();
    }

    std::string MerchantAgent::GetTradeGreeting() const
    {
        return "Take a look at my shop.";
    }

    std::string MerchantAgent::GetTradeClosedMessage() const
    {
        return "Sorry, shop's closed.";
    }

    void MerchantAgent::ClearOffers()
    {
        mOffers.clear();
    }

    void MerchantAgent::AddOffer(const TradeOffer& offer)
    {
        mOffers.push_back(offer);
    }
}
