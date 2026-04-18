/**
 * @file TestTrading.cpp
 * @brief Tests for player gold, enemy gold drops, and merchant trading flow.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/Classic/Enemy.hpp"
#include "../../source/Agents/Classic/MerchantAgent.hpp"
#include "../../source/Agents/Classic/PlayerAgent.hpp"
#include "../../source/core/WorldBase.hpp"

namespace cse498 {
class TestWorld : public WorldBase {
public:
    TestWorld() : WorldBase() {
        // KAREN: Create the player here to avoid interfering with other groups' demos (temp fix)
        auto p = std::make_unique<PlayerAgent>(GetNextAgentId(), "Player", *this);
        AddAgent(std::move(p));
        mPlayer = dynamic_cast<PlayerAgent*>(agent_set[0].get());
        assert(mPlayer);
    }

    ~TestWorld() override = default;

    int DoAction([[maybe_unused]] AgentBase& agent, [[maybe_unused]] size_t action_id) override { return 0; }
};

class TestItem : public Item {
public:
    TestItem(size_t id, std::string name, int goldValue, const WorldBase& world) :
        Item(id, std::move(name), "path", goldValue, world) {}
};
} // namespace cse498

using namespace cse498;
static void SetupBasicMerchant(MerchantAgent& merchant) {
    merchant.ClearInitialOffers();
    merchant.AddInitialOffer({"apple", 4, 2, 1, TradeStockMode::Unlimited, 0});
    merchant.AddInitialOffer({"bread", 6, 3, 1, TradeStockMode::Limited, 18});
    merchant.AddInitialOffer({"potion", 10, 5, 1, TradeStockMode::Limited, 10});
    merchant.AddGold(200);
}

TEST_CASE("PlayerAgent gold helpers work", "[agents][player][gold]") {
    TestWorld world;
    PlayerAgent player(100, "Tester", world);

    REQUIRE(player.GetGold() == 0);

    player.AddGold(15);
    REQUIRE(player.GetGold() == 15);

    REQUIRE(player.SpendGold(10));
    REQUIRE(player.GetGold() == 5);

    REQUIRE_FALSE(player.SpendGold(6));
    REQUIRE(player.GetGold() == 5);

    player.SetGold(42);
    REQUIRE(player.GetGold() == 42);
}

TEST_CASE("Enemy gold drop can only be claimed once until reset", "[agents][enemy][gold]") {
    TestWorld world;
    Enemy enemy(200, "Slime", world);

    REQUIRE(enemy.GetGoldDrop() == 5);

    enemy.SetGoldDrop(9);
    REQUIRE(enemy.GetGoldDrop() == 9);

    REQUIRE(enemy.ClaimGoldDrop() == 9);
    REQUIRE(enemy.ClaimGoldDrop() == 0);

    enemy.ResetGoldDropClaim();
    REQUIRE(enemy.ClaimGoldDrop() == 9);
}

TEST_CASE("Buying a limited merchant item reduces stock and player gold", "[agents][trade][buy]") {
    TestWorld world;
    PlayerAgent player(300, "Buyer", world);
    MerchantAgent merchant(301, "Merchant", world);
    SetupBasicMerchant(merchant);

    player.SetGold(100);

    const TradeOffer* before = merchant.FindOffer("bread");
    REQUIRE(before != nullptr);
    REQUIRE_FALSE(before->IsUnlimited());
    REQUIRE(before->mStock == 18);

    TradeResult result = merchant.BuyFromMerchant(player, "bread", 2);

    REQUIRE(result.IsSuccess());
    REQUIRE(result.mStatus == TradeStatus::Success);
    REQUIRE(result.mGoldMoved == 12);

    REQUIRE(player.GetGold() == 88);
    REQUIRE(player.GetInventory().GetTotal("bread") == 2);

    const TradeOffer* after = merchant.FindOffer("bread");
    REQUIRE(after != nullptr);
    REQUIRE(after->mStock == 16);
    REQUIRE(merchant.GetGold() == 212);
}

TEST_CASE("Selling unknown item creates limited merchant offer", "[agents][trade][sell][new-offer]") {
    TestWorld world;
    PlayerAgent player(400, "Seller", world);
    MerchantAgent merchant(401, "Merchant", world);
    SetupBasicMerchant(merchant);

    player.SetGold(0);

    REQUIRE(merchant.FindOffer("ruby") == nullptr);

    player.GetInventory().AddItem(std::make_unique<TestItem>(1, "ruby", 8, world), 2);
    REQUIRE(player.GetInventory().GetTotal("ruby") == 2);

    TradeResult result = merchant.SellToMerchant(player, "ruby", 2);

    REQUIRE(result.IsSuccess());
    REQUIRE(result.mStatus == TradeStatus::Success);
    REQUIRE(result.mGoldMoved == 8);

    REQUIRE(player.GetGold() == 8);
    REQUIRE(player.GetInventory().GetTotal("ruby") == 0);

    const TradeOffer* offer = merchant.FindOffer("ruby");
    REQUIRE(offer != nullptr);
    REQUIRE_FALSE(offer->IsUnlimited());
    REQUIRE(offer->mStock == 2);
    REQUIRE(offer->mBuyPrice == 8);
    REQUIRE(offer->mSellPrice == 4);
}

TEST_CASE("Selling existing limited item increases merchant stock", "[agents][trade][sell][restock]") {
    TestWorld world;
    PlayerAgent player(500, "Seller", world);
    MerchantAgent merchant(501, "Merchant", world);
    SetupBasicMerchant(merchant);

    player.SetGold(0);

    const TradeOffer* before = merchant.FindOffer("bread");
    REQUIRE(before != nullptr);
    REQUIRE(before->mStock == 18);

    player.GetInventory().AddItem(std::make_unique<TestItem>(2, "bread", 6, world), 3);
    REQUIRE(player.GetInventory().GetTotal("bread") == 3);

    TradeResult result = merchant.SellToMerchant(player, "bread", 3);

    REQUIRE(result.IsSuccess());
    REQUIRE(result.mStatus == TradeStatus::Success);
    REQUIRE(result.mGoldMoved == 9);

    REQUIRE(player.GetGold() == 9);
    REQUIRE(player.GetInventory().GetTotal("bread") == 0);

    const TradeOffer* after = merchant.FindOffer("bread");
    REQUIRE(after != nullptr);
    REQUIRE(after->mStock == 21);
}

TEST_CASE("Buying with quantity 0 fails", "[TradeSystem][buy][edge]") {
    // Create a merchant with known stock and give the player gold.
    TestWorld world;
    PlayerAgent player(601, "Player", world);
    MerchantAgent merchant(602, "Merchant", world);
    SetupBasicMerchant(merchant);

    player.SetGold(100);

    // Attempt to buy zero items.
    const TradeResult result = merchant.BuyFromMerchant(player, "bread", 0);

    // Assert transaction is rejected and no state changes occur.
    REQUIRE_FALSE(result.IsSuccess());
    REQUIRE(result.mStatus == TradeStatus::InvalidQuantity);
    REQUIRE(player.GetGold() == 100);

    const TradeOffer* bread = merchant.FindOffer("bread");
    REQUIRE(bread != nullptr);
    REQUIRE(bread->mStock == 18);
}

TEST_CASE("Buying an unknown item fails", "[TradeSystem][buy][unknown]") {
    // Merchant only knows about the configured default shop items.
    TestWorld world;
    PlayerAgent player(611, "Player", world);
    MerchantAgent merchant(612, "Merchant", world);
    SetupBasicMerchant(merchant);

    player.SetGold(100);

    // Ask to buy an item that is not in the merchant catalog.
    const TradeResult result = merchant.BuyFromMerchant(player, "ruby", 1);

    // Assert merchant rejects the request and nothing changes.
    REQUIRE_FALSE(result.IsSuccess());
    REQUIRE(result.mStatus == TradeStatus::UnknownItem);
    REQUIRE(player.GetGold() == 100);
}

TEST_CASE("Buying without enough gold fails", "[TradeSystem][buy][gold]") {
    // Player cannot afford the requested purchase.
    TestWorld world;
    PlayerAgent player(621, "Player", world);
    MerchantAgent merchant(622, "Merchant", world);
    SetupBasicMerchant(merchant);

    player.SetGold(2);

    const TradeOffer* before = merchant.FindOffer("bread");
    REQUIRE(before != nullptr);
    REQUIRE(before->mStock == 18);

    // Try to buy one bread costing more than the player's gold.
    const TradeResult result = merchant.BuyFromMerchant(player, "bread", 1);

    // Assert purchase fails and stock/gold stay unchanged.
    REQUIRE_FALSE(result.IsSuccess());
    REQUIRE(result.mStatus == TradeStatus::InsufficientFunds);
    REQUIRE(player.GetGold() == 2);

    const TradeOffer* after = merchant.FindOffer("bread");
    REQUIRE(after != nullptr);
    REQUIRE(after->mStock == 18);
}

TEST_CASE("Buying more than merchant stock fails", "[TradeSystem][buy][stock]") {
    // bread is limited stock, so requesting too much should fail.
    TestWorld world;
    PlayerAgent player(631, "Player", world);
    MerchantAgent merchant(632, "Merchant", world);
    SetupBasicMerchant(merchant);

    player.SetGold(1000);

    const TradeOffer* before = merchant.FindOffer("bread");
    REQUIRE(before != nullptr);
    REQUIRE(before->mStock == 18);

    // Try to buy more bread than exists in stock.
    const TradeResult result = merchant.BuyFromMerchant(player, "bread", 25);

    // Assert transaction fails and limited stock is unchanged.
    REQUIRE_FALSE(result.IsSuccess());
    REQUIRE(result.mStatus == TradeStatus::MerchantOutOfStock);
    REQUIRE(player.GetGold() == 1000);

    const TradeOffer* after = merchant.FindOffer("bread");
    REQUIRE(after != nullptr);
    REQUIRE(after->mStock == 18);
}

TEST_CASE("Selling with quantity 0 fails", "[TradeSystem][sell][edge]") {
    // Player has an item, but quantity 0 should still be rejected.
    TestWorld world;
    PlayerAgent player(641, "Player", world);
    MerchantAgent merchant(642, "Merchant", world);
    SetupBasicMerchant(merchant);

    player.GetInventory().AddItem(std::make_unique<Item>(0, "bread", "", 6, world), 1);

    // Try to sell zero copies.
    const TradeResult result = merchant.SellToMerchant(player, "bread", 0);

    // Assert transaction is rejected immediately.
    REQUIRE_FALSE(result.IsSuccess());
    REQUIRE(result.mStatus == TradeStatus::InvalidQuantity);
}

TEST_CASE("Selling an item the player does not own fails", "[TradeSystem][sell][missing]") {
    // Merchant is valid, but player inventory does not contain the item.
    TestWorld world;
    PlayerAgent player(651, "Player", world);
    MerchantAgent merchant(652, "Merchant", world);
    SetupBasicMerchant(merchant);

    // Try to sell an item that is not in the player's inventory.
    const TradeResult result = merchant.SellToMerchant(player, "ruby", 1);

    // Assert merchant rejects the sale.
    REQUIRE_FALSE(result.IsSuccess());
    REQUIRE(result.mStatus == TradeStatus::PlayerOutOfStock);
}

TEST_CASE("Selling fails when merchant cannot afford payout", "[TradeSystem][sell][merchant-gold]") {
    // Player owns bread, but merchant has no gold.
    TestWorld world;
    PlayerAgent player(661, "Player", world);
    MerchantAgent merchant(662, "Merchant", world);
    SetupBasicMerchant(merchant);

    merchant.SetGold(0);
    player.GetInventory().AddItem(std::make_unique<Item>(0, "bread", "", 6, world), 1);

    // Try to sell bread back to the merchant.
    const TradeResult result = merchant.SellToMerchant(player, "bread", 1);

    // Assert sale fails because merchant cannot pay.
    REQUIRE_FALSE(result.IsSuccess());
    REQUIRE(result.mStatus == TradeStatus::MerchantCannotAfford);

    // Player should still have the item.
    REQUIRE(player.GetInventory().GetTotal("bread") == 1);
}

TEST_CASE("Closed merchant blocks buying and selling", "[TradeSystem][merchant][closed]") {
    // Merchant exists but trading is disabled.
    TestWorld world;
    PlayerAgent player(671, "Player", world);
    MerchantAgent merchant(672, "Merchant", world);
    SetupBasicMerchant(merchant);

    merchant.SetAvailableForTrade(false);
    player.SetGold(100);
    player.GetInventory().AddItem(std::make_unique<Item>(0, "bread", "", 6, world), 1);

    // Attempt both buy and sell operations.
    const TradeResult buyResult = merchant.BuyFromMerchant(player, "bread", 1);
    const TradeResult sellResult = merchant.SellToMerchant(player, "bread", 1);

    // Assert both operations are rejected because the merchant is closed.
    REQUIRE_FALSE(buyResult.IsSuccess());
    REQUIRE(buyResult.mStatus == TradeStatus::MerchantClosed);

    REQUIRE_FALSE(sellResult.IsSuccess());
    REQUIRE(sellResult.mStatus == TradeStatus::MerchantClosed);
}
