/**
 * @file TestFarmingAgent.cpp
 * @brief Unit tests for Farming Agent behavior
 */


#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/Classic/FarmingAgent.hpp"
#include "../../source/core/WorldBase.hpp"



namespace cse498
{
    /**
     * Minimal dummy world to allow agent construction.
     */
    class TestWorld : public WorldBase
    {
    public:
        TestWorld() : WorldBase() {}
        ~TestWorld() override = default;

        int DoAction([[maybe_unused]] AgentBase& agent, [[maybe_unused]] size_t action_id) override
        {
            return 0;
        }
    };
}

using namespace cse498;
static void SetupBasicFarmer(FarmingAgent& farmer)
{
    farmer.ClearInitialOffers();
    farmer.AddInitialOffer({"wheat", 3, 1, 1, TradeStockMode::Limited, 30});
    farmer.AddInitialOffer({"seeds", 2, 1, 1, TradeStockMode::Unlimited, 0});
    farmer.AddInitialOffer({"carrot", 4, 2, 1, TradeStockMode::Limited, 18});
    farmer.AddGold(100);
}

TEST_CASE("FarmingAgent initializes correctly", "[FarmingAgent]")
{
    TestWorld world;
    FarmingAgent agent(1, "Farmer", world);

    REQUIRE(agent.GetName() == "Farmer");
    REQUIRE(agent.IsAvailableForTrade() == true);
}

TEST_CASE("FarmingAgent SelectAction returns 0", "[FarmingAgent]")
{
    TestWorld world;
    FarmingAgent agent(2, "Farmer", world);

    WorldGrid grid;
    REQUIRE(agent.SelectAction(grid) == 0);
}

TEST_CASE("FarmingAgent trade availability toggle", "[FarmingAgent]")
{
    TestWorld world;
    FarmingAgent farmer(3, "Farmer", world);

    REQUIRE(farmer.IsAvailableForTrade());

    farmer.SetAvailableForTrade(false);
    REQUIRE_FALSE(farmer.IsAvailableForTrade());

    farmer.SetAvailableForTrade(true);
    REQUIRE(farmer.IsAvailableForTrade());
}

TEST_CASE("FarmingAgent has expected trade greeting", "[FarmingAgent][greeting]")
{
    TestWorld world;
    FarmingAgent farmer(4, "Farmer", world);

    REQUIRE(farmer.GetTradeGreeting() == "Fresh crops and seeds today.");
}

TEST_CASE("FarmingAgent can be configured with expected offers", "[FarmingAgent][offers]")
{
    TestWorld world;
    FarmingAgent farmer(5, "Farmer", world);
    SetupBasicFarmer(farmer);

    const auto& offers = farmer.GetOffers();
    REQUIRE(offers.size() == 3);

    const TradeOffer* wheat = farmer.FindOffer("wheat");
    const TradeOffer* seeds = farmer.FindOffer("seeds");
    const TradeOffer* carrot = farmer.FindOffer("carrot");

    REQUIRE(wheat != nullptr);
    REQUIRE(seeds != nullptr);
    REQUIRE(carrot != nullptr);

    SECTION("wheat offer is limited with expected values")
    {
        REQUIRE_FALSE(wheat->IsUnlimited());
        REQUIRE(wheat->mStockMode == TradeStockMode::Limited);
        REQUIRE(wheat->mStock == 30);
        REQUIRE(wheat->mBuyPrice == 3);
        REQUIRE(wheat->mSellPrice == 1);
        REQUIRE(wheat->mItemValue == 1);
    }

    SECTION("seeds offer is unlimited with expected values")
    {
        REQUIRE(seeds->IsUnlimited());
        REQUIRE(seeds->mStockMode == TradeStockMode::Unlimited);
        REQUIRE(seeds->mStock == 0);
        REQUIRE(seeds->mBuyPrice == 2);
        REQUIRE(seeds->mSellPrice == 1);
        REQUIRE(seeds->mItemValue == 1);
    }

    SECTION("carrot offer is limited with expected values")
    {
        REQUIRE_FALSE(carrot->IsUnlimited());
        REQUIRE(carrot->mStockMode == TradeStockMode::Limited);
        REQUIRE(carrot->mStock == 18);
        REQUIRE(carrot->mBuyPrice == 4);
        REQUIRE(carrot->mSellPrice == 2);
        REQUIRE(carrot->mItemValue == 1);
    }
}

TEST_CASE("Trade Greeting Setting and Getting")
{
    TestWorld world;
    FarmingAgent farmer(5, "Farmer", world);
    farmer.SetTradeGreeting("TestFun");
    farmer.SetTradeClosedMessage("Test1");
    CHECK(farmer.GetTradeGreeting() == "TestFun");
    CHECK(farmer.GetTradeClosedMessage() == "Test1");


}