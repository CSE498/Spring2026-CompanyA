/**
 * @file TestFetchAgent.cpp
 * @brief Unit tests for the generic FetchAgent flow.
 */

// used codex 5.4 for test generation

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/AI/FetchAgent.hpp"
#include "../../source/Worlds/Hub/Building.hpp"
#include "../../source/Worlds/Hub/InteractiveWorld.hpp"
#include "../../source/Worlds/Hub/ResourceBank.hpp"
#include "../../source/Worlds/Hub/ResourceSpawn.hpp"
#include "../../source/Worlds/Hub/TownHall.hpp"

using namespace cse498;

namespace {

void StepAgent(InteractiveWorld& world, FetchAgent& agent) {
    const std::size_t action = agent.SelectAction(world.GetGrid());
    world.DoAction(agent, action);
}

} // namespace

TEST_CASE("FetchAgent routes between generic origin and deposit points", "[FetchAgent][generic]") {
    InteractiveWorld world;

    Building& origin = world.AddAgent<Building>("Origin");
    Building& deposit = world.AddAgent<Building>("Deposit");
    world.AddBuilding(origin, WorldPosition{4, 2});
    world.AddBuilding(deposit, WorldPosition{8, 2});

    FetchAgent& agent = world.AddAgent<FetchAgent>("Hauler");
    agent.SetOrigin(origin).SetDepositPoint(deposit);
    agent.SetPosition(WorldPosition{2, 2});

    const std::size_t firstAction = agent.SelectAction(world.GetGrid());
    REQUIRE(firstAction == agent.GetActionID("right"));

    world.DoAction(agent, firstAction);
    REQUIRE(agent.GetPosition() == WorldPosition(3, 2));

    StepAgent(world, agent);
    REQUIRE(agent.GetCarryQuantity() == 1);

    agent.SetPosition(WorldPosition{7, 2});
    StepAgent(world, agent);
    REQUIRE(agent.GetCarryQuantity() == 0);
    REQUIRE(agent.GetTotalDelivered() == 1);
}

TEST_CASE("FetchAgent supports resource hauling through endpoint callbacks", "[FetchAgent][resources]") {
    InteractiveWorld world;

    auto townHallPtr = std::make_unique<TownHall>(world.GetNextAgentId(), "Town Hall", world, world.GetInventoryPtr());
    TownHall& townHall = world.AddAgent(std::move(townHallPtr));
    world.AddTownHall(townHall, WorldPosition{8, 4});

    auto spawnPtr = std::make_unique<ResourceSpawn>(world.GetNextAgentId(), "Wood Spawn", world, ItemType::Wood);
    ResourceSpawn& spawn = world.AddAgent(std::move(spawnPtr));
    spawn.AddResource(5);
    world.AddResourceSpawn(spawn, WorldPosition{4, 4});

    FetchAgent& agent = world.AddAgent<FetchAgent>("Wood Hauler");
    agent.SetOrigin(spawn)
            .SetDepositPoint(townHall)
            .SetItemType(spawn.GetItemType())
            .SetOnOriginReached([&spawn](FetchAgent& fetcher) {
                fetcher.SetItemType(spawn.GetItemType());
                fetcher.SetCarryQuantity(spawn.Collect());
            })
            .SetOnDepositReached([&townHall](FetchAgent& fetcher) {
                const int quantity = fetcher.GetCarryQuantity();
                if (quantity <= 0) {
                    return;
                }

                townHall.DepositResource(fetcher.GetItemType(), quantity);
                fetcher.AddDelivered(quantity);
                fetcher.SetCarryQuantity(0);
            });

    agent.SetPosition(WorldPosition{3, 4});
    StepAgent(world, agent);

    REQUIRE(agent.GetCarryQuantity() == 5);
    REQUIRE(spawn.GetQuantity() == 0);

    agent.SetPosition(WorldPosition{7, 4});
    StepAgent(world, agent);

    REQUIRE(agent.GetCarryQuantity() == 0);
    REQUIRE(agent.GetTotalDelivered() == 5);
    REQUIRE(world.GetInventory().GetAmount(ItemType::Wood) == 5);
}

TEST_CASE("FetchAgent waits at an empty ResourceSpawn until resources appear", "[FetchAgent][resources]") {
    InteractiveWorld world;

    auto spawnPtr = std::make_unique<ResourceSpawn>(world.GetNextAgentId(), "Wood Spawn", world, ItemType::Wood);
    ResourceSpawn& spawn = world.AddAgent(std::move(spawnPtr));
    world.AddResourceSpawn(spawn, WorldPosition{4, 2});

    ResourceBank& bank = world.AddAgent<ResourceBank>("Bank");
    world.AddResourceBank(bank, WorldPosition{8, 2});

    FetchAgent& agent = world.AddAgent<FetchAgent>("Wood Hauler");
    agent.SetOrigin(spawn).SetDepositPoint(bank).SetPosition(WorldPosition{3, 2});

    StepAgent(world, agent);
    REQUIRE(agent.GetCarryQuantity() == 0);
    REQUIRE(agent.GetPosition() == WorldPosition(3, 2));

    const std::size_t waitingAction = agent.SelectAction(world.GetGrid());
    REQUIRE(waitingAction == 0);

    spawn.AddResource(4);
    StepAgent(world, agent);

    REQUIRE(agent.GetItemType() == ItemType::Wood);
    REQUIRE(agent.GetCarryQuantity() == 4);
    REQUIRE(spawn.GetQuantity() == 0);
}
