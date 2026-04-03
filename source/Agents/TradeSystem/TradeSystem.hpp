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
        /**
         * Creates a generic item instance for trade insertion or rollback
         *
         * @param nextItemId Counter used to assign unique item ids
         * @param world Owning world reference for item
         * @param itemName Name of item to make
         * @param value Gold value metadata for item
         * @return Newly created item instance
         */
        static std::unique_ptr<Item> MakeItem(
            std::size_t& nextItemId,
            const WorldBase& world,
            const std::string& itemName,
            int value);
    public:
        /**
         * Executes a purchase from merchant to player
         *
         * @param world Owning world reference
         * @param nextItemId Counter used to assign unique item ids
         * @param player performing purchase
         * @param merchant fulfilling the purchase
         * @param offer Offer being purchased
         * @param quantity requested
         * @return Result of transaction
         */
        static TradeResult BuyItem(
            const WorldBase& world,
            std::size_t& nextItemId,
            PlayerAgent& player,
            MerchantAgent& merchant,
            TradeOffer& offer,
            std::size_t quantity);

        /**
         * Executes a sale from player to merchant
         *
         * @param world Owning world reference
         * @param nextItemId Counter used to assign unique item ids
         * @param player performing the sale
         * @param merchant receiving the sale
         * @param offer Offer being restocked or updated
         * @param quantity Quantity requested
         * @return Result of the transaction
         */
        static TradeResult SellItem(
            const WorldBase& world,
            std::size_t& nextItemId,
            PlayerAgent& player,
            MerchantAgent& merchant,
            TradeOffer& offer,
            std::size_t quantity);
    };
} // cse498
