/**
 * @file TestFarmingAgent.cpp
 * @brief Unit tests for Farming Agent behavior
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/Classic/FarmingAgent.hpp"
#include "../../source/Agents/Classic/PlayerAgent.hpp"
#include "../../source/core/WorldBase.hpp"

namespace cse498 {

/**
 * Minimal dummy world to allow agent construction.
 *
 * FarmingAgent only needs a valid owning world for these unit tests,
 * so this world keeps behavior intentionally simple.
 */
class TestWorld : public WorldBase {
public:
    TestWorld() : WorldBase() {
        // Create a player since there are a lot of assumptions that one exists
        auto p = std::make_unique<PlayerAgent>(GetNextAgentId(), "Player", *this);
        AddAgent(std::move(p));
        mPlayer = dynamic_cast<PlayerAgent*>(agent_set[0].get());
        assert(mPlayer);
    }

    ~TestWorld() override = default;

    // Test world doesn't simulate real actions but rather just satisfies the abstract interface
    int DoAction([[maybe_unused]] AgentBase& agent, [[maybe_unused]] size_t action_id) override { return 0; }
};

} // namespace cse498

using namespace cse498;

/**
 * Gives the farmer a basic shop setup similar to an actual farming merchant.
 *
 * Includes: limited wheat, unlimited seeds, limited carrots
 */
static void SetupBasicFarmer(FarmingAgent& farmer) {
    farmer.ClearInitialOffers();
    farmer.AddInitialOffer({"wheat", 3, 1, 1, TradeStockMode::Limited, 30});
    farmer.AddInitialOffer({"seeds", 2, 1, 1, TradeStockMode::Unlimited, 0});
    farmer.AddInitialOffer({"carrot", 4, 2, 1, TradeStockMode::Limited, 18});
    farmer.AddGold(100);
}

/**
 * Registers the basic movement/interact actions used by FarmingAgent tests.
 */
static void AddBasicFarmerActions(FarmingAgent& farmer) {
    farmer.AddAction("up", 1);
    farmer.AddAction("down", 2);
    farmer.AddAction("left", 3);
    farmer.AddAction("right", 4);
    farmer.AddAction("interact", 5);
}

/**
 * Builds a simple fully walkable test grid.
 *
 * This is useful for testing farming behavior in isolation without walls
 * affecting the farmer's movement/path selection.
 */
static WorldGrid MakeOpenFloorGrid(std::size_t width = 8, std::size_t height = 8) {
    WorldGrid grid(width, height);
    const std::size_t floorId = grid.AddCellType("floor", "walkable floor", '.');

    for (std::size_t y = 0; y < height; ++y) {
        for (std::size_t x = 0; x < width; ++x) {
            grid[x, y] = floorId;
        }
    }

    return grid;
}

TEST_CASE("FarmingAgent initializes correctly", "[FarmingAgent]") {
    TestWorld world;
    FarmingAgent agent(1, "Farmer", world);

    // Confirm default values are set as expected
    REQUIRE(agent.GetName() == "Farmer");
    REQUIRE(agent.IsAvailableForTrade() == true);
    REQUIRE(agent.GetTradeGreeting() == "Fresh crops and seeds today.");
    REQUIRE(agent.GetFarmerState() == FarmingAgent::FarmerState::IdleAtHome);
}

TEST_CASE("FarmingAgent SelectAction returns 0 when no building is assigned", "[FarmingAgent]") {
    TestWorld world;
    FarmingAgent agent(2, "Farmer", world);
    AddBasicFarmerActions(agent);

    WorldGrid grid = MakeOpenFloorGrid();

    // Without being assigned a building, farmer shouldn't begin work cycle
    REQUIRE(agent.SelectAction(grid) == 0);
}

TEST_CASE("FarmingAgent trade availability toggle", "[FarmingAgent]") {
    TestWorld world;
    FarmingAgent farmer(3, "Farmer", world);

    // Merchant should begin as available for trade by default
    REQUIRE(farmer.IsAvailableForTrade());

    farmer.SetAvailableForTrade(false);
    REQUIRE_FALSE(farmer.IsAvailableForTrade());

    farmer.SetAvailableForTrade(true);
    REQUIRE(farmer.IsAvailableForTrade());
}

TEST_CASE("FarmingAgent can be configured with expected offers", "[FarmingAgent][offers]") {
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

    SECTION("wheat offer is limited with expected values") {
        // Wheat should be a restockable limited-stock item.
        REQUIRE_FALSE(wheat->IsUnlimited());
        REQUIRE(wheat->mStockMode == TradeStockMode::Limited);
        REQUIRE(wheat->mStock == 30);
        REQUIRE(wheat->mBuyPrice == 3);
        REQUIRE(wheat->mSellPrice == 1);
        REQUIRE(wheat->mItemValue == 1);
    }

    SECTION("seeds offer is unlimited with expected values") {
        // Seeds should remain an always-available offer.
        REQUIRE(seeds->IsUnlimited());
        REQUIRE(seeds->mStockMode == TradeStockMode::Unlimited);
        REQUIRE(seeds->mStock == 0);
        REQUIRE(seeds->mBuyPrice == 2);
        REQUIRE(seeds->mSellPrice == 1);
        REQUIRE(seeds->mItemValue == 1);
    }

    SECTION("carrot offer is limited with expected values") {
        REQUIRE_FALSE(carrot->IsUnlimited());
        REQUIRE(carrot->mStockMode == TradeStockMode::Limited);
        REQUIRE(carrot->mStock == 18);
        REQUIRE(carrot->mBuyPrice == 4);
        REQUIRE(carrot->mSellPrice == 2);
        REQUIRE(carrot->mItemValue == 1);
    }
}

TEST_CASE("FarmingAgent stores assigned building, home, and work settings", "[FarmingAgent][config]") {
    TestWorld world;
    FarmingAgent farmer(6, "Farmer", world);
    Building building(7, "Farm", world);

    building.SetPosition(WorldPosition(4, 2));
    farmer.SetAssignedBuilding(&building);
    farmer.SetHomePosition(WorldPosition(1, 1));
    farmer.SetWorkInterval(5);
    farmer.SetRestockAmount(3);
    farmer.SetRestockItemName("carrot");

    // Verify all basic farming configuration is stored correctly
    REQUIRE(farmer.GetAssignedBuilding() == &building);
    REQUIRE(farmer.GetHomePosition() == WorldPosition(1, 1));
    REQUIRE(farmer.GetWorkInterval() == 5);
    REQUIRE(farmer.GetRestockAmount() == 3);
    REQUIRE(farmer.GetRestockItemName() == "carrot");
}

TEST_CASE("FarmingAgent starts moving toward assigned building when work is due", "[FarmingAgent][work]") {
    TestWorld world;
    FarmingAgent farmer(8, "Farmer", world);
    Building building(9, "Farm", world);

    AddBasicFarmerActions(farmer);

    WorldGrid grid = MakeOpenFloorGrid();

    farmer.SetPosition(WorldPosition(1, 1));
    farmer.SetHomePosition(WorldPosition(1, 1));
    building.SetPosition(WorldPosition(4, 1));

    farmer.SetAssignedBuilding(&building);
    farmer.SetWorkInterval(0);

    const std::size_t action = farmer.SelectAction(grid);

    // When time to work, farmer should start heading toward the building.
    REQUIRE(action == farmer.GetActionID("right"));
    REQUIRE(farmer.GetFarmerState() == FarmingAgent::FarmerState::GoingToWork);
}

TEST_CASE("FarmingAgent interacts when already adjacent to assigned building", "[FarmingAgent][interact]") {
    TestWorld world;
    FarmingAgent farmer(10, "Farmer", world);
    Building building(11, "Farm", world);

    AddBasicFarmerActions(farmer);

    WorldGrid grid = MakeOpenFloorGrid();

    farmer.SetPosition(WorldPosition(3, 1));
    farmer.SetHomePosition(WorldPosition(1, 1));
    building.SetPosition(WorldPosition(4, 1));

    farmer.SetAssignedBuilding(&building);
    farmer.SetWorkInterval(0);

    const std::size_t action = farmer.SelectAction(grid);

    // If the farmer is already next to building, work should begin immediately
    REQUIRE(action == farmer.GetActionID("interact"));
    REQUIRE(farmer.GetFarmerState() == FarmingAgent::FarmerState::Working);
}

TEST_CASE("OnWorkedAtBuilding restocks limited offer and starts return home", "[FarmingAgent][restock]") {
    TestWorld world;
    FarmingAgent farmer(12, "Farmer", world);
    Building building(13, "Farm", world);

    SetupBasicFarmer(farmer);

    farmer.SetAssignedBuilding(&building);
    farmer.SetRestockItemName("wheat");
    farmer.SetRestockAmount(2);

    // Add an upgrade so level 1 is valid
    building.AddUpgrade(ItemType::Wood, 25);
    REQUIRE(building.SetCurrentLevel(1));

    const TradeOffer* before = farmer.FindOffer("wheat");
    REQUIRE(before != nullptr);
    REQUIRE(before->mStock == 30);

    farmer.OnWorkedAtBuilding();

    const TradeOffer* after = farmer.FindOffer("wheat");
    REQUIRE(after != nullptr);
    REQUIRE(after->mStock == 33); // 30 + (2 + level 1)
    REQUIRE(farmer.GetFarmerState() == FarmingAgent::FarmerState::ReturningHome);
}

TEST_CASE("FarmingAgent returns home after working", "[FarmingAgent][return-home]") {
    TestWorld world;
    FarmingAgent farmer(14, "Farmer", world);
    Building building(15, "Farm", world);

    AddBasicFarmerActions(farmer);

    WorldGrid grid = MakeOpenFloorGrid();

    farmer.SetHomePosition(WorldPosition(1, 1));
    farmer.SetPosition(WorldPosition(3, 1));
    building.SetPosition(WorldPosition(4, 1));
    farmer.SetAssignedBuilding(&building);

    farmer.OnWorkedAtBuilding();

    SECTION("farmer moves toward home while returning") {
        // After work, farmer should start heading home
        const std::size_t action = farmer.SelectAction(grid);
        REQUIRE(action == farmer.GetActionID("left"));
        REQUIRE(farmer.GetFarmerState() == FarmingAgent::FarmerState::ReturningHome);
    }

    SECTION("farmer becomes idle once back home") {
        // Once home, farmer should go idle.
        farmer.SetPosition(WorldPosition(1, 1));
        const std::size_t action = farmer.SelectAction(grid);
        REQUIRE(action == 0);
        REQUIRE(farmer.GetFarmerState() == FarmingAgent::FarmerState::IdleAtHome);
    }
}

TEST_CASE("FarmingAgent does not restock unlimited offers", "[FarmingAgent][restock]") {
    TestWorld world;
    FarmingAgent farmer(17, "Farmer", world);
    Building building(18, "Farm", world);

    SetupBasicFarmer(farmer);

    building.AddUpgrade(ItemType::Wood, 25);
    REQUIRE(building.SetCurrentLevel(1));

    farmer.SetAssignedBuilding(&building);
    farmer.SetRestockItemName("seeds");   // seeds is unlimited
    farmer.SetRestockAmount(5);

    const TradeOffer* before = farmer.FindOffer("seeds");
    REQUIRE(before != nullptr);
    REQUIRE(before->mStockMode == TradeStockMode::Unlimited);
    REQUIRE(before->mStock == 0);

    farmer.OnWorkedAtBuilding();

    const TradeOffer* after = farmer.FindOffer("seeds");
    REQUIRE(after != nullptr);

    // Unlimited offers should not gain stock from RestockLimitedOffer().
    REQUIRE(after->mStock == 0);
    REQUIRE(farmer.GetFarmerState() == FarmingAgent::FarmerState::ReturningHome);
}

TEST_CASE("FarmingAgent returns 0 when no walkable adjacent work tile exists", "[FarmingAgent][work]") {
    TestWorld world;
    FarmingAgent farmer(19, "Farmer", world);
    Building building(20, "Farm", world);

    AddBasicFarmerActions(farmer);

    // Build a grid where every tile is blocked by default.
    WorldGrid grid(7, 7);
    const std::size_t wallId = grid.AddCellType("wall", "blocked", '#');

    for (std::size_t y = 0; y < 7; ++y) {
        for (std::size_t x = 0; x < 7; ++x) {
            grid[x, y] = wallId;
        }
    }

    farmer.SetPosition(WorldPosition(1, 1));
    farmer.SetHomePosition(WorldPosition(1, 1));
    building.SetPosition(WorldPosition(3, 3));

    farmer.SetAssignedBuilding(&building);
    farmer.SetWorkInterval(0);

    const std::size_t action = farmer.SelectAction(grid);

    // If there is no valid adjacent tile next to the building, farmer cannot path to work.
    REQUIRE(action == 0);
}

TEST_CASE("Trade Greeting Setting and Getting", "[FarmingAgent][messages]") {
    TestWorld world;
    FarmingAgent farmer(16, "Farmer", world);
    farmer.SetTradeGreeting("TestFun");
    farmer.SetTradeClosedMessage("Test1");
    CHECK(farmer.GetTradeGreeting() == "TestFun");
    CHECK(farmer.GetTradeClosedMessage() == "Test1");
}