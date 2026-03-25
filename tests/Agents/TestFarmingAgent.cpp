/**
* @file TestFarmingAgent.cpp
 * @brief Unit tests for Farming Agent behavior
 */


#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/FarmingAgent.hpp"
#include "../../source/core/WorldBase.hpp"

using namespace cse498;

/**
 * Minimal dummy world to allow agent construction.
 */
class TestWorld : public WorldBase
{
public:
    int DoAction(AgentBase &, size_t) override
    {
        return 0;
    }
};

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
    FarmingAgent agent(1, "Farmer", world);

    WorldGrid grid;
    REQUIRE(agent.SelectAction(grid) == 0);
}

TEST_CASE("FarmingAgent trade availability toggle", "[FarmingAgent]")
{
    TestWorld world;
    FarmingAgent agent(1, "Farmer", world);

    REQUIRE(agent.IsAvailableForTrade() == true);

    agent.SetAvailableForTrade(false);
    REQUIRE(agent.IsAvailableForTrade() == false);

    agent.SetAvailableForTrade(true);
    REQUIRE(agent.IsAvailableForTrade() == true);
}