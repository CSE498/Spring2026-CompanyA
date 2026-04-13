/**
 * @file TradeSystem.cpp
 * @author Ty Maksimowski
 */

#include "TradeSystem.hpp"

#include "../../../core/WorldBase.hpp"
#include "../MerchantAgent.hpp"
#include "../PlayerAgent.hpp"

namespace cse498
{
    TradeResult TradeSystem::BuyItem(const WorldBase& world, PlayerAgent& player,
        MerchantAgent& merchant, TradeOffer& offer, std::size_t quantity)
    {
        return BuyItem<Item>(player, merchant, offer, quantity, 0, offer.mItemName, "", offer.mItemValue, world);
    }

    TradeResult TradeSystem::SellItem(const WorldBase& world, PlayerAgent& player,
        MerchantAgent& merchant, TradeOffer& offer, std::size_t quantity)
    {
        return SellItem<Item>(player, merchant, offer, quantity, 0, offer.mItemName, "", offer.mItemValue, world);
    }
}
