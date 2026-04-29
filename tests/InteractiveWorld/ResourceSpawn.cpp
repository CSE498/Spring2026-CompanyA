#include "../../source/Worlds/Hub/InteractiveWorld.hpp"
#include "../../source/Worlds/Hub/ResourceSpawn.hpp"
#include "catch2/catch.hpp"

using namespace cse498;

TEST_CASE("ResourceSpawn rejects invalid resource and collection limits", "[core][InteractiveWorld][ResourceSpawn]") {
    InteractiveWorld world;
    ResourceSpawn spawn(1, "Wood Spawn", world, ItemType::Wood);

    CHECK(spawn.GetItemType() == ItemType::Wood);
    CHECK(spawn.GetQuantity() == 0);
    CHECK(spawn.GetMaxCollectionQuantity() == 10);

    spawn.AddResource(-5);
    spawn.AddResource(0);
    CHECK(spawn.GetQuantity() == 0);

    spawn.AddResource(12);
    CHECK(spawn.GetQuantity() == 12);

    spawn.SetMaxCollectionQuantity(-2);
    spawn.SetMaxCollectionQuantity(0);
    CHECK(spawn.GetMaxCollectionQuantity() == 10);

    spawn.SetMaxCollectionQuantity(4);
    CHECK(spawn.GetMaxCollectionQuantity() == 4);
    CHECK(spawn.Collect() == 4);
    CHECK(spawn.GetQuantity() == 8);
}

