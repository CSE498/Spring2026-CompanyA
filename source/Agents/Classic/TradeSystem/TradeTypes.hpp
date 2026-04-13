/**
 * @file TradeTypes.hpp
 * @author Ty Maksimowski
 *
 * Simple trade data types for merchant/player transactions.
 */

#pragma once

#include <cstddef>
#include <string>

namespace cse498
{
    /**
     * Determines whether an offer has finite or infinite stock
     */
    enum class TradeStockMode
    {
        Unlimited,
        Limited
    };

    /**
     * One merchant offer.
     *
     * For Unlimited offers, mStock is ignored.
     * For Limited offers, mStock tracks remaining merchant stock.
     */
    struct TradeOffer
    {
        std::string mItemName;
        std::size_t mBuyPrice = 1;
        std::size_t mSellPrice = 1;
        int mItemValue = 1;
        TradeStockMode mStockMode = TradeStockMode::Unlimited;
        std::size_t mStock = 0;

        [[nodiscard]] bool IsUnlimited() const
        {
            return mStockMode == TradeStockMode::Unlimited;
        }

        [[nodiscard]] bool HasEnough(std::size_t quantity) const
        {
            return IsUnlimited() || mStock >= quantity;
        }

        /**
         * Consume stock for a purchase.
         * @return true if stock was consumed or offer is unlimited.
         */
        bool Consume(std::size_t quantity)
        {
            if (IsUnlimited()) return true;
            if (mStock < quantity) return false;
            mStock -= quantity;
            return true;
        }

        /**
         * Return stock back to the merchant.
         * Useful for rollback or for simple buyback behavior.
         */
        void Restock(std::size_t quantity)
        {
            if (!IsUnlimited()) mStock += quantity;
        }
    };

    enum class TradeStatus
    {
        Success,
        InvalidQuantity,
        MerchantClosed,
        UnknownItem,
        MerchantOutOfStock,
        PlayerOutOfStock,
        InsufficientFunds,
        MerchantCannotAfford,
        InventoryFull
    };

    struct TradeResult
    {
        TradeStatus mStatus = TradeStatus::Success;
        std::string mMessage;
        std::string mItemName;
        std::size_t mQuantity = 0;
        std::size_t mGoldMoved = 0;

        [[nodiscard]] bool IsSuccess() const
        {
            return mStatus == TradeStatus::Success;
        }
    };
}
