/**
 * @file TradeSystem.cpp
 * @author Ty Maksimowski
 */

#include "TradeSystem.hpp"

#include "../../core/WorldBase.hpp"
#include "../MerchantAgent.hpp"
#include "../PlayerAgent.hpp"

namespace cse498
{
    std::unique_ptr<Item> TradeSystem::MakeItem(
        std::size_t& nextItemId,
        const WorldBase& world,
        const std::string& itemName,
        int value)
    {
        return std::make_unique<Item>(nextItemId++, itemName, "", value, world);
    }

    TradeResult TradeSystem::BuyItem(const WorldBase& world, std::size_t& nextItemId, PlayerAgent& player,
        MerchantAgent& merchant, TradeOffer& offer, std::size_t quantity)
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

        if (playerInventory.AddItem(MakeItem(nextItemId, world, offer.mItemName, offer.mItemValue), quantity) != 0)
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

    TradeResult TradeSystem::SellItem(const WorldBase& world, std::size_t& nextItemId, PlayerAgent& player,
        MerchantAgent& merchant, TradeOffer& offer, std::size_t quantity)
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
            playerInventory.AddItem(MakeItem(nextItemId, world, offer.mItemName, offer.mItemValue), quantity);

            return {
                TradeStatus::MerchantCannotAfford,
                "Merchant gold changed. Try again.",
                offer.mItemName,
                quantity,
                0
            };
        }

        player.AddGold(payout);

        /**
         * Selling to the merchant makes the item available for repurchase.
         * Unlimited offers ignore this, limited offers increase stock.
         */
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
}
