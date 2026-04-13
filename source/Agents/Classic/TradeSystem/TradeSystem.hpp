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
#include <utility>

#include "TradeTypes.hpp"
#include "../PlayerFeatures/Inventory.hpp"
#include "../MerchantAgent.hpp"
#include "../PlayerAgent.hpp"

namespace cse498
{
    class WorldBase;
    class MerchantAgent;

    class TradeSystem
    {
    public:
        /**
         * Non-template convenience overload for basic merchant purchases.
         *
         * Preserves simple non-template trade API used by current merchant call sites.
         * For now, default fallback type is the base Item class, but more specialized subclasses
         * can be supported later.
         *
         * @param world Owning world reference
         * @param player performing purchase
         * @param merchant fulfilling the purchase
         * @param offer Offer being purchased
         * @param quantity requested
         * @return Result of transaction
         */
        static TradeResult BuyItem(const WorldBase& world, PlayerAgent& player, MerchantAgent& merchant,
            TradeOffer& offer, std::size_t quantity);

        /**
         * Non-template convenience overload for basic merchant sales.
         *
         * Preserves simple non-template trade API used by current merchant call sites.
         * For now, default fallback type is the base Item class, but more specialized subclasses
         * can be supported later.
         *
         * @param world Owning world reference
         * @param player performing the sale
         * @param merchant receiving the sale
         * @param offer Offer being restocked or updated
         * @param quantity Quantity requested
         * @return Result of the transaction
         */
        static TradeResult SellItem(const WorldBase& world, PlayerAgent& player, MerchantAgent& merchant, TradeOffer& offer, std::size_t quantity);

        /**
         * Executes a purchase from merchant to player using a specific item type.
         *
         * This templated overload routes item construction through Inventory::AddItem<ITEM_T>(), the forwarded
         * constructor arguments should match the constructor expected by ITEM_T
         *
         * @tparam ITEM_T Item type to construct and add
         * @tparam Args Constructor argument types forwarded into ITEM_T
         * @param player Player performing purchase
         * @param merchant Merchant fulfilling purchase
         * @param offer Offer being purchased
         * @param quantity Quantity requested
         * @param args Constructor arguments forwarded to ITEM_T
         * @return Result of transaction
         */
        template<typename ITEM_T, typename... Args>
        requires std::derived_from<ITEM_T, Item>
        static TradeResult BuyItem(PlayerAgent& player, MerchantAgent& merchant, TradeOffer& offer,
                                    std::size_t quantity, Args&&... args)
        {
            Inventory& playerInventory = player.GetInventory();

            if (quantity == 0)
            {
                return {
                    TradeStatus::InvalidQuantity,
                    "Quantity must be at least 1.",
                    offer.mItemName,
                    0,
                    0
                };
            }

            if (!offer.HasEnough(quantity))
            {
                return {
                    TradeStatus::MerchantOutOfStock,
                    "Merchant is out of stock.",
                    offer.mItemName,
                    quantity,
                    0
                };
            }

            const std::size_t totalCost = offer.mBuyPrice * quantity;
            if (player.GetGold() < totalCost)
            {
                return {
                    TradeStatus::InsufficientFunds,
                    "You do not have enough gold.",
                    offer.mItemName,
                    quantity,
                    totalCost
                };
            }

            if (!player.SpendGold(totalCost))
            {
                return {
                    TradeStatus::InsufficientFunds,
                    "Could not remove player gold.",
                    offer.mItemName,
                    quantity,
                    totalCost
                };
            }

            if (!offer.Consume(quantity))
            {
                player.AddGold(totalCost);
                return {
                    TradeStatus::MerchantOutOfStock,
                    "Merchant stock changed. Try again.",
                    offer.mItemName,
                    quantity,
                    0
                };
            }

            // Use Inventory's typed construction path for trade item creation
            if (playerInventory.AddItem<ITEM_T>(quantity, std::forward<Args>(args)...) != 0)
            {
                offer.Restock(quantity);
                player.AddGold(totalCost);

                return {
                    TradeStatus::InventoryFull,
                    "Your inventory is full.",
                    offer.mItemName,
                    quantity,
                    0
                };
            }

            merchant.AddGold(totalCost);

            return {
                TradeStatus::Success,
                "Bought " + std::to_string(quantity) + " " + offer.mItemName +
                    " for " + std::to_string(totalCost) + " gold.",
                offer.mItemName,
                quantity,
                totalCost
            };
        }

        /**
         * Executes a sale from player to merchant using a specific item type.
         *
         * This templated overload is mainly used for rollback when sold items must be reinserted into
         * the player's inventory after a failed merchant gold transaction. It routes reconstruction through
         * Inventory::AddItem<ITEM_T>() using forwarded constructor arguments, which should match the constructor
         * expected by ITEM_T.
         *
         * @tparam ITEM_T Item type to construct and add
         * @tparam Args Constructor argument types forwarded into ITEM_T
         * @param player Player performing the sale
         * @param merchant Merchant receiving sale
         * @param offer Offer being restocked or updated
         * @param quantity Quantity requested
         * @param args Constructor argument types forwarded into ITEM_T
         * @return Result of the transaction
         */
        template<typename ITEM_T, typename... Args>
        requires std::derived_from<ITEM_T, Item>
        static TradeResult SellItem(PlayerAgent& player, MerchantAgent& merchant, TradeOffer& offer,
                                    std::size_t quantity, Args&&... args)
        {
            Inventory& playerInventory = player.GetInventory();

            if (quantity == 0)
            {
                return {
                    TradeStatus::InvalidQuantity,
                    "Quantity must be at least 1.",
                    offer.mItemName,
                    0,
                    0
                };
            }

            if (playerInventory.GetTotal(offer.mItemName) < quantity)
            {
                return {
                    TradeStatus::PlayerOutOfStock,
                    "You do not have enough to sell.",
                    offer.mItemName,
                    quantity,
                    0
                };
            }

            const std::size_t payout = offer.mSellPrice * quantity;
            if (merchant.GetGold() < payout)
            {
                return {
                    TradeStatus::MerchantCannotAfford,
                    "Merchant does not have enough gold.",
                    offer.mItemName,
                    quantity,
                    payout
                };
            }
            // != 0 means quantity wasn't removed at all and user doesn't have the stuffs
            if (playerInventory.RemoveItem(offer.mItemName, quantity, true) != 0)
            {
                return {
                    TradeStatus::PlayerOutOfStock,
                    "Could not remove items from your inventory.",
                    offer.mItemName,
                    quantity,
                    0
                };
            }

            if (!merchant.SpendGold(payout))
            {
                // Reconstruct sold item through Inventory's typed construction path if sale needs to be rolled back
                playerInventory.AddItem<ITEM_T>(quantity, std::forward<Args>(args)...);

                return {
                    TradeStatus::MerchantCannotAfford,
                    "Merchant gold changed. Try again.",
                    offer.mItemName,
                    quantity,
                    0
                };
            }

            player.AddGold(payout);
            offer.Restock(quantity);

            return {
                TradeStatus::Success,
                "Sold " + std::to_string(quantity) + " " + offer.mItemName +
                    " for " + std::to_string(payout) + " gold.",
                offer.mItemName,
                quantity,
                payout
            };
        }
    };
} // cse498
