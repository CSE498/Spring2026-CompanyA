/**
 * @file ResourceBank.cpp
 * @brief Unit tests for the ResourceBank class.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Worlds/Hub/InteractiveWorld.hpp"
#include "../../source/Worlds/Hub/ResourceBank.hpp"

using namespace cse498;

TEST_CASE("ResourceBank starts empty", "[core][InteractiveWorld][ResourceBank]") {
    InteractiveWorld world;
    ResourceBank bank(1, "Bank", world);

    CHECK(bank.GetStoredAmount(ItemType::Wood) == 0);
    CHECK(bank.GetStoredAmount(ItemType::Stone) == 0);
    CHECK(bank.GetStoredAmount(ItemType::Metal) == 0);
    CHECK(bank.GetInventory().Empty());
}

TEST_CASE("ResourceBank stores and withdraws resources", "[core][InteractiveWorld][ResourceBank]") {
    InteractiveWorld world;
    ResourceBank bank(2, "Bank", world);

    CHECK(bank.DepositResource(ItemType::Wood, 12));
    CHECK(bank.DepositResource(ItemType::Stone, 4));
    CHECK(bank.GetStoredAmount(ItemType::Wood) == 12);
    CHECK(bank.GetStoredAmount(ItemType::Stone) == 4);
    CHECK(bank.HasEnough(ItemType::Wood, 10));

    CHECK(bank.WithdrawResource(ItemType::Wood, 5));
    CHECK(bank.GetStoredAmount(ItemType::Wood) == 7);
    CHECK_FALSE(bank.HasEnough(ItemType::Wood, 8));

    CHECK(bank.WithdrawResource(ItemType::Stone, 4));
    CHECK(bank.GetStoredAmount(ItemType::Stone) == 0);
}

TEST_CASE("ResourceBank rejects invalid transfers", "[core][InteractiveWorld][ResourceBank]") {
    InteractiveWorld world;
    ResourceBank bank(3, "Bank", world);

    CHECK_FALSE(bank.DepositResource(ItemType::Metal, -1));
    CHECK_FALSE(bank.WithdrawResource(ItemType::Metal, -1));
    CHECK(bank.DepositResource(ItemType::Metal, 0));
    CHECK(bank.WithdrawResource(ItemType::Metal, 0));
    CHECK_FALSE(bank.WithdrawResource(ItemType::Metal, 1));

    CHECK(bank.DepositResource(ItemType::Metal, 999));
    CHECK_FALSE(bank.DepositResource(ItemType::Metal, 1));
    CHECK(bank.GetStoredAmount(ItemType::Metal) == 999);
}

TEST_CASE("InteractiveWorld can place a ResourceBank on the map", "[core][InteractiveWorld][ResourceBank]") {
    InteractiveWorld world;
    ResourceBank& bank = world.AddAgent<ResourceBank>("Bank");

    world.AddResourceBank(bank, WorldPosition{6, 4});

    CHECK(bank.GetLocation().AsWorldPosition() == WorldPosition(6, 4));
    CHECK_FALSE(world.GetGrid().IsWalkable(WorldPosition(6, 4)));
}
