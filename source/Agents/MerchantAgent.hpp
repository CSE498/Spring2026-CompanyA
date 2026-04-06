/**
 * @file MerchantAgent.hpp
 * @author Ty Maksimowski
 *
 * Basic merchant NPC that can support trading with the player.
 */

#pragma once
#include <cstddef>
#include <string>
#include <vector>

#include "../core/AgentBase.hpp"
#include "TradeSystem/TradeTypes.hpp"

namespace cse498
{
    class PlayerAgent;

    class MerchantAgent : public AgentBase
    {
    private:
        /** Controls whether merchant currently accepts trades */
        bool mAvailableForTrade = true;

        /** Catalog of items merchant can trade*/
        std::vector<TradeOffer> mOffers;

        /** Currency held by merchant for paying the player for sales. */
        std::size_t mGold = 0;

    protected:
        /**
         * Removes all current offers from the merchant catalog.
         */
        void ClearOffers();

        /**
         * Adds a new offer to the merchant catalog.
         *
         * @param offer Offer to add
         */
        void AddOffer(const TradeOffer& offer);

        /**
         * Finds a mutable offer by the item name.
         *
         * @param itemName Name of offered item
         * @return Pointer to matching offer, or nullptr if not found
         */
        [[nodiscard]] TradeOffer* FindOfferMutable(const std::string& itemName);

        /**
         * Ensures that a limited-stock offer exists for given item.
         *
         * If the item already exists in the merchant's catalog, that offer is returned.
         * Otherwise, a new limited offer is created so an item sold to the merchant by the
         * player can appear in the shop and be bought back later.
         *
         * @param itemName Name of the item
         * @param buyPrice Price player has to pay to buy item
         * @param sellPrice Price the merchant pays to buy the item
         * @param itemValue Stored value metadata for recreated items
         * @return Reference to the existing or newly created offer
         */
        TradeOffer& EnsureLimitedOffer(const std::string& itemName, std::size_t buyPrice,
                                       std::size_t sellPrice, int itemValue);

    public:
        /**
         * Constructs an empty merchant agent.
         *
         * Initial shop offers and starting gold are expected to be provided by owning world.
         *
         * @param id Unique agent id
         * @param name Display name
         * @param world Owning world
         */
        MerchantAgent(std::size_t id, const std::string& name, WorldBase& world);

        /**
         * Merchant agents are passive and do not choose actions.
         *
         * @param grid Current world grid snapshot
         * @return 0
         */
        [[nodiscard]] std::size_t SelectAction(const WorldGrid& grid) override;

        /**
         * @return True if merchant is accepting trades
         */
        [[nodiscard]] bool IsAvailableForTrade() const;

        /**
         * Enables or disables trading.
         *
         * @param available New trading availability state
         */
        void SetAvailableForTrade(bool available);

        /**
         * @return Read-only view of the merchant's trade offers
         */
        [[nodiscard]] const std::vector<TradeOffer>& GetOffers() const;

        /**
         * Finds an offer by item name
         *
         * @param itemName Name of the offered item
         * @return Pointer to matching offer, or nullptr if not found
         */
        [[nodiscard]] const TradeOffer* FindOffer(const std::string& itemName) const;

        /**
         * Adds an initial shop offer during world setup.
         *
         * This is intended for setup time, so owning world can decide what the merchant sells without
         * hardcoding it in the constructor.
         *
         * @param offer Offer to add to merchant catalog
         */
        void AddInitialOffer(const TradeOffer& offer);

        /**
         * Removes all existing shop offers during world setup.
         */
        void ClearInitialOffers();

        /**
         * @return Merchant's current gold total
         */
        [[nodiscard]] std::size_t GetGold() const { return mGold; }

        /**
         * Adds gold to the merchant
         *
         * @param amount Gold to add
         */
        void AddGold(std::size_t amount) { mGold += amount; }

        void SetGold(std::size_t amount) { mGold = amount; }

        /**
         * Attempts to remove gold from merchant.
         *
         * @param amount Gold to spend
         * @return True if enough gold to spend
         */
        bool SpendGold(std::size_t amount);

        /**
         * Attempts to buy an item from the merchant into the player's inventory.
         *
         * @param player Player performing the purchase
         * @param itemName Name of the item to buy
         * @param quantity Quantity requested
         * @return Result describing success or failure and gold movement
         */
        TradeResult BuyFromMerchant(PlayerAgent& player, const std::string& itemName, std::size_t quantity = 1);

        /**
         * Attempts to sell an item from the player to the merchant. If the item is not already
         * sold by the merchant, a new limited offer is created so the item can appear in the shop afterward.
         *
         * @param player Player performing the sale
         * @param itemName Name of the item to sell
         * @param quantity Quantity requested
         * @return Result describing success or failure and gold movement
         */
        TradeResult SellToMerchant(PlayerAgent& player, const std::string& itemName, std::size_t quantity = 1);

        /**
         * @return Greeting shown when the player opens the trade menu
         */
        [[nodiscard]] virtual std::string GetTradeGreeting() const;

        /**
         * @return Message shown when the merchant shop is closed
         */
        [[nodiscard]] virtual std::string GetTradeClosedMessage() const;
    };
} // cse498
