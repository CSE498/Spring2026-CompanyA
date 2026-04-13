/**
 * @file InteractiveWorldInventory.cpp
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/Worlds/Hub/InteractiveWorldInventory.hpp"

using namespace cse498;

TEST_CASE("Inventory starts empty", "[core][InteractiveWorld]") {
    InteractiveWorldInventory inv;

    CHECK(inv.Empty() == true);
    CHECK(inv.GetAmount(ItemType::Wood) == 0);
    CHECK(inv.GetAmount(ItemType::Stone) == 0);
    CHECK(inv.GetAmount(ItemType::Metal) == 0);
}

TEST_CASE("AddItem increases resource amount", "[core][InteractiveWorld]") {
    InteractiveWorldInventory inv;

    CHECK(inv.AddItem(ItemType::Wood, 10) == true);
    CHECK(inv.Empty() == false);
    CHECK(inv.GetAmount(ItemType::Wood) == 10);

    CHECK(inv.AddItem(ItemType::Wood, 5) == true);
    CHECK(inv.GetAmount(ItemType::Wood) == 15);

    CHECK(inv.AddItem(ItemType::Stone, 3) == true);
    CHECK(inv.GetAmount(ItemType::Stone) == 3);
}

TEST_CASE("RemoveItem decreases resource when enough exists", "[core][InteractiveWorld]") {
    InteractiveWorldInventory inv;

    CHECK(inv.AddItem(ItemType::Stone, 8) == true);

    CHECK(inv.RemoveItem(ItemType::Stone, 3) == true);
    CHECK(inv.GetAmount(ItemType::Stone) == 5);

    CHECK(inv.RemoveItem(ItemType::Stone, 5) == true);
    CHECK(inv.GetAmount(ItemType::Stone) == 0);
    CHECK(inv.Empty() == true);
}

TEST_CASE("RemoveItem fails when not enough resources", "[core][InteractiveWorld]") {
    InteractiveWorldInventory inv;

    CHECK(inv.AddItem(ItemType::Metal, 2) == true);

    CHECK(inv.RemoveItem(ItemType::Metal, 5) == false);
    CHECK(inv.GetAmount(ItemType::Metal) == 2);
}

TEST_CASE("RemoveItem fails on fresh inventory", "[core][InteractiveWorld]") {
    InteractiveWorldInventory inv;

    CHECK(inv.RemoveItem(ItemType::Wood, 1) == false);
    CHECK(inv.RemoveItem(ItemType::Stone, 1) == false);
    CHECK(inv.RemoveItem(ItemType::Metal, 1) == false);

    CHECK(inv.Empty() == true);
}

TEST_CASE("HasEnough returns correct values", "[core][InteractiveWorld]") {
    InteractiveWorldInventory inv;

    CHECK(inv.AddItem(ItemType::Wood, 7) == true);

    CHECK(inv.HasEnough(ItemType::Wood, 3) == true);
    CHECK(inv.HasEnough(ItemType::Wood, 7) == true);
    CHECK(inv.HasEnough(ItemType::Wood, 8) == false);

    CHECK(inv.HasEnough(ItemType::Stone, 1) == false);
}

TEST_CASE("Clear removes all resources", "[core][InteractiveWorld]") {
    InteractiveWorldInventory inv;

    CHECK(inv.AddItem(ItemType::Wood, 5) == true);
    CHECK(inv.AddItem(ItemType::Stone, 10) == true);
    CHECK(inv.AddItem(ItemType::Metal, 15) == true);

    CHECK(inv.Empty() == false);

    inv.Clear();

    CHECK(inv.Empty() == true);
    CHECK(inv.GetAmount(ItemType::Wood) == 0);
    CHECK(inv.GetAmount(ItemType::Stone) == 0);
    CHECK(inv.GetAmount(ItemType::Metal) == 0);
}

TEST_CASE("Different resources do not affect each other", "[core][InteractiveWorld]") {
    InteractiveWorldInventory inv;

    CHECK(inv.AddItem(ItemType::Wood, 10) == true);
    CHECK(inv.AddItem(ItemType::Stone, 5) == true);

    CHECK(inv.RemoveItem(ItemType::Wood, 4) == true);

    CHECK(inv.GetAmount(ItemType::Wood) == 6);
    CHECK(inv.GetAmount(ItemType::Stone) == 5);
}

TEST_CASE("Removing exact amount makes inventory empty", "[core][InteractiveWorld]") {
    InteractiveWorldInventory inv;

    CHECK(inv.AddItem(ItemType::Metal, 6) == true);
    CHECK(inv.RemoveItem(ItemType::Metal, 6) == true);

    CHECK(inv.GetAmount(ItemType::Metal) == 0);
    CHECK(inv.Empty() == true);
}

TEST_CASE("AddItem with zero amount does not add resources", "[core][InteractiveWorld]") {
    InteractiveWorldInventory inv;

    CHECK(inv.AddItem(ItemType::Wood, 0) == true);
    CHECK(inv.GetAmount(ItemType::Wood) == 0);

    // Depending on implementation, Empty() should ideally still be true.
    // If this fails, the class may be inserting zero-count entries.
    CHECK(inv.Empty() == true);
}
