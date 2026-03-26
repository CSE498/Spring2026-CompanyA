/**
 * @file TradeSystem.hpp
 * @author Ty Maksimowski
 *
 * Transaction helpers for merchant/player trading.
 */

#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include "TradeTypes.hpp"
#include "../PlayerFeatures/Inventory.hpp"

namespace cse498
{
    class WorldBase;
    class PlayerAgent;
    class MerchantAgent;

    class TradeSystem
    {
    private:
        static std::unique_ptr<Item> MakeItem(
            std::size_t& nextItemId,
            const WorldBase& world,
            const std::string& itemName,
            int value);
    public:
        static TradeResult BuyItem(
            const WorldBase& world,
            std::size_t& nextItemId,
            PlayerAgent& player,
            MerchantAgent& merchant,
            TradeOffer& offer,
            std::size_t quantity);

        static TradeResult SellItem(
            const WorldBase& world,
            std::size_t& nextItemId,
            PlayerAgent& player,
            MerchantAgent& merchant,
            TradeOffer& offer,
            std::size_t quantity);
    };
} // cse498
