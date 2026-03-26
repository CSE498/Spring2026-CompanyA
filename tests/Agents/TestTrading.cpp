/**
 * @file TestTrading.cpp
 * @brief Tests for player gold, enemy gold drops, and merchant trading flow.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/Enemy.hpp"
#include "../../source/Agents/MerchantAgent.hpp"
#include "../../source/Agents/PlayerAgent.hpp"
#include "../../source/core/WorldBase.hpp"

namespace cse498
{
    class TestWorld : public WorldBase
    {
    public:
        TestWorld() : WorldBase()
        {
        }

        ~TestWorld() override = default;

        int DoAction([[maybe_unused]] AgentBase& agent,
                     [[maybe_unused]] size_t action_id) override
        {
            return 0;
        }
    };

    class TestItem : public Item
    {
    public:
        TestItem(size_t id,
                 std::string name,
                 int goldValue,
                 const WorldBase& world)
            : Item(id, std::move(name), "path", goldValue, world)
        {
        }
    };
}

using namespace cse498;

TEST_CASE("PlayerAgent gold helpers work", "[agents][player][gold]")
{
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

TEST_CASE("Enemy gold drop can only be claimed once until reset", "[agents][enemy][gold]")
{
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

TEST_CASE("Buying a limited merchant item reduces stock and player gold", "[agents][trade][buy]")
{
    TestWorld world;
    PlayerAgent player(300, "Buyer", world);
    MerchantAgent merchant(301, "Merchant", world);

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

TEST_CASE("Selling unknown item creates limited merchant offer", "[agents][trade][sell][new-offer]")
{
    TestWorld world;
    PlayerAgent player(400, "Seller", world);
    MerchantAgent merchant(401, "Merchant", world);

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

TEST_CASE("Selling existing limited item increases merchant stock", "[agents][trade][sell][restock]")
{
    TestWorld world;
    PlayerAgent player(500, "Seller", world);
    MerchantAgent merchant(501, "Merchant", world);

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
