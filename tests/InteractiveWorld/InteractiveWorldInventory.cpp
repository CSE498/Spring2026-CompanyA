/**
 * @file InteractiveWorldInventory.cpp
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/core/InteractiveWorld/InteractiveWorldInventory.hpp"

using namespace cse498;

TEST_CASE("Inventory starts empty") {

    InteractiveWorldInventory inv;

    CHECK(inv.Empty() == true);

    CHECK(inv.GetAmount(ItemType::Wood) == 0);
    CHECK(inv.GetAmount(ItemType::Stone) == 0);
    CHECK(inv.GetAmount(ItemType::Metal) == 0);
}

TEST_CASE("AddItem increases resource amount") {

    InteractiveWorldInventory inv;

    inv.AddItem(ItemType::Wood, 10);

    CHECK(inv.Empty() == false);
    CHECK(inv.GetAmount(ItemType::Wood) == 10);

    inv.AddItem(ItemType::Wood, 5);

    CHECK(inv.GetAmount(ItemType::Wood) == 15);

    inv.AddItem(ItemType::Stone, 3);

    CHECK(inv.GetAmount(ItemType::Stone) == 3);
}

TEST_CASE("RemoveItem decreases resource when enough exists") {

    InteractiveWorldInventory inv;

    inv.AddItem(ItemType::Stone, 8);

    CHECK(inv.RemoveItem(ItemType::Stone, 3) == true);

    CHECK(inv.GetAmount(ItemType::Stone) == 5);

    CHECK(inv.RemoveItem(ItemType::Stone, 5) == true);

    CHECK(inv.GetAmount(ItemType::Stone) == 0);
}

TEST_CASE("RemoveItem fails when not enough resources") {

    InteractiveWorldInventory inv;

    inv.AddItem(ItemType::Metal, 2);

    CHECK(inv.RemoveItem(ItemType::Metal, 5) == false);

    CHECK(inv.GetAmount(ItemType::Metal) == 2);
}

TEST_CASE("HasEnough returns correct values") {

    InteractiveWorldInventory inv;

    inv.AddItem(ItemType::Wood, 7);

    CHECK(inv.HasEnough(ItemType::Wood, 3) == true);

    CHECK(inv.HasEnough(ItemType::Wood, 7) == true);

    CHECK(inv.HasEnough(ItemType::Wood, 8) == false);

    CHECK(inv.HasEnough(ItemType::Stone, 1) == false);
}

TEST_CASE("Clear removes all resources") {

    InteractiveWorldInventory inv;

    inv.AddItem(ItemType::Wood, 5);
    inv.AddItem(ItemType::Stone, 10);
    inv.AddItem(ItemType::Metal, 15);

    CHECK(inv.Empty() == false);

    inv.Clear();

    CHECK(inv.Empty() == true);

    CHECK(inv.GetAmount(ItemType::Wood) == 0);
    CHECK(inv.GetAmount(ItemType::Stone) == 0);
    CHECK(inv.GetAmount(ItemType::Metal) == 0);
}

TEST_CASE("Different resources do not affect each other") {

    InteractiveWorldInventory inv;

    inv.AddItem(ItemType::Wood, 10);
    inv.AddItem(ItemType::Stone, 5);

    inv.RemoveItem(ItemType::Wood, 4);

    CHECK(inv.GetAmount(ItemType::Wood) == 6);
    CHECK(inv.GetAmount(ItemType::Stone) == 5);
}

TEST_CASE("Removing exact amount makes inventory empty") {

    InteractiveWorldInventory inv;

    inv.AddItem(ItemType::Metal, 6);

    CHECK(inv.RemoveItem(ItemType::Metal, 6) == true);

    CHECK(inv.Empty() == true);
}