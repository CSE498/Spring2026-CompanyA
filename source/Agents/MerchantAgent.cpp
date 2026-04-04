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
        // Merchant starts empty by default and owning world decides what merchant sells and how
        // much gold it starts with
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
        // Search the offer list by item name
        const auto it = std::ranges::find_if(mOffers,
                                             [&itemName](const TradeOffer& offer)
                                             {
                                                 return offer.mItemName == itemName;
                                             });

        return (it == mOffers.end()) ? nullptr : &(*it);
    }

    void MerchantAgent::AddInitialOffer(const TradeOffer& offer)
    {
        AddOffer(offer);
    }

    void MerchantAgent::ClearInitialOffers()
    {
        ClearOffers();
    }

    bool MerchantAgent::SpendGold(const std::size_t amount)
    {
        // reject the transaction if merchant doesn't have enough
        if (amount > mGold)
        {
            return false;
        }

        mGold -= amount;
        return true;
    }

    TradeResult MerchantAgent::BuyFromMerchant(PlayerAgent& player, const std::string& itemName, std::size_t quantity)
    {
        // Stop if merchant isn't trading
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

        // Find if the offer is in the merchant's offer list
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

        // delegate transaction rules to TradeSystem
        return TradeSystem::BuyItem(
            GetWorld(),
            player,
            *this,
            *offer,
            quantity
        );
    }

    TradeResult MerchantAgent::SellToMerchant(PlayerAgent& player, const std::string& itemName, std::size_t quantity)
    {
        // stop if merchant isn't trading
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

        // can't sell 0 of something to a merchant
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

        // Inspect the item from the player's inventory so pricing can be derived from item itself
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

        // get a buy and sell price for shop
        const int itemValue = item->GetGold();
        const std::size_t buyPrice = static_cast<std::size_t>(std::max(1, itemValue));
        const std::size_t sellPrice = std::max<std::size_t>(1, buyPrice / 2);

        // make sure the sold item shows up in the shop as a limited offer
        TradeOffer& offer = EnsureLimitedOffer(item->GetName(), buyPrice, sellPrice, itemValue);

        return TradeSystem::SellItem(
            GetWorld(),
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
        // Reuse the existing offer if the merchant already sells this item
        if (TradeOffer* existing = FindOfferMutable(itemName); existing != nullptr)
        {
            return *existing;
        }

        // create a new limited offer entry so the player can buy it back later
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
