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
        const WorldBase& world,
        const std::string& itemName,
        int value)
    {
        // For non-unique trade items, inventory ignores item id, so 0 is used instead of generating a unique id
        return std::make_unique<Item>(0, itemName, "", value, world);
    }

    TradeResult TradeSystem::BuyItem(const WorldBase& world, PlayerAgent& player,
        MerchantAgent& merchant, TradeOffer& offer, std::size_t quantity)
    {
        // Work with player's inventory during transaction
        Inventory& playerInventory = player.GetInventory();

        // Reject invalid quantities
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

        // Make sure merchant has enough
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

        // compute total price for requested quantity
        const std::size_t totalCost = offer.mBuyPrice * quantity;
        // player needs to have enough gold
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

        // spend gold first before inventory changes
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

        // reserve limited stock before adding the item to the player's inventory
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

        // Roll back cost and stock if the item cannot be inserted into inventory
        if (playerInventory.AddItem(MakeItem(world, offer.mItemName, offer.mItemValue), quantity) != 0)
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

        // Transfer gold to the merchant after the item has been added successfully
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

    TradeResult TradeSystem::SellItem(const WorldBase& world, PlayerAgent& player,
        MerchantAgent& merchant, TradeOffer& offer, std::size_t quantity)
    {
        // Get player's inventory to work with during transaction
        Inventory& playerInventory = player.GetInventory();

        // reject invalid quantities
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

        // Player has to own enough of the item to sell it
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

        // Compute how much gold merchant owes
        const std::size_t payout = offer.mSellPrice * quantity;
        // Merchant needs enough gold to purchase
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

        // Remove item from the player's inventory before paying out gold owed
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

        // If merchant can't spend gold after removal, need to restore sold item
        if (!merchant.SpendGold(payout))
        {
            playerInventory.AddItem(MakeItem(world, offer.mItemName, offer.mItemValue), quantity);

            return {
                TradeStatus::MerchantCannotAfford,
                "Merchant gold changed. Try again.",
                offer.mItemName,
                quantity,
                0
            };
        }

        // Pay the player for the item
        player.AddGold(payout);

        // Restock the offer so sold items can be bought back
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
