/**
 * @file TestInventory.cpp
 * @author Logan Rimarcik
 */

#include "../../source/Agents/Classic/PlayerFeatures/Inventory.hpp"
#include "../../source/core/WorldBase.hpp"
#include "../../source/core/item/Item.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

namespace cse498 {
class MockWorldBase : public WorldBase {
public:
    MockWorldBase() : WorldBase() {
        // KAREN: Create the player here to avoid interfering with other groups' demos (temp fix)
        auto p = std::make_unique<PlayerAgent>(GetNextAgentId(), "Player", *this);
        AddAgent(std::move(p));
        mPlayer = dynamic_cast<PlayerAgent*>(agent_set[0].get());
        assert(mPlayer);
    }
    ~MockWorldBase() override = default;
    int DoAction([[maybe_unused]] AgentBase& agent, [[maybe_unused]] size_t action_id) override { return 0; }
};

/**
 * Shared test world used by mock items so they do not hold references
 * to temporary WorldBase objects that immediately go out of scope.
 */
inline MockWorldBase& GetTestWorld() {
    static MockWorldBase world;
    return world;
}

class MockTestItem : public Item {
public:
    MockTestItem(size_t id = 0, std::string name = "TestItem") : Item(id, std::move(name), "path", 4, GetTestWorld()) {}
};
class MockTestItem2 : public Item {
public:
    MockTestItem2(size_t id = 0, std::string name = "TestItem") :
        Item(id, std::move(name), "path", 4, GetTestWorld()) {}
    [[nodiscard]] bool IsUnique() const override { return true; }
};

// This is a 'real' item mimicking real items that is 'U' = unique
class RealUItem : public Item {
public:
    RealUItem(size_t id, int gold, const WorldBase& world) : Item(id, "RealUItem", "path", gold, world) {}
    [[nodiscard]] bool IsUnique() const override { return true; }
};
// This is a 'real' item that is non-unique (gold price is determined)
class RealItem : public Item {
public:
    RealItem(size_t id, const WorldBase& world) : Item(id, "RealItem", "path", 4, world) {}
};
} // namespace cse498

using cse498::Inventory;
using cse498::MockTestItem;
using cse498::MockTestItem2;
using IS = Inventory::InventorySlot;
using cse498::MockWorldBase;
using cse498::RealItem;
using cse498::RealUItem;

TEST_CASE("Inventory Slot Constructors", "[Constructor]") {
    {
        IS x{};
        CHECK(!x.GetItem());
    }
    {
        IS x{std::make_unique<MockTestItem>(), 3};
        CHECK(x.GetItem());
        CHECK(x.GetQuantity() == 3);
    }
}

TEST_CASE("Reset and Getters", "[inventoryslot, getters]") {
    // NOTE: range of valid input is [1,64] otherwise assert triggers
    {
        IS x{std::make_unique<MockTestItem>(), 3};
        x.Reset();
        CHECK(!x.GetItem());
        CHECK(x.GetQuantity() == 0);
        CHECK(!x.IsFull());
    }
    {
        IS x{std::make_unique<MockTestItem>(), 64};
        CHECK(x.GetQuantity() == 64);
        CHECK(x.IsFull());
        CHECK(x.GetItem());
        x.Reset();
        CHECK(x.GetQuantity() == 0);
        CHECK(x.IsEmpty());
        CHECK(!x.GetItem());
    }
}

TEST_CASE("Increment/Decrement Quantity", "[inventoryslot]") {
    {
        IS x{std::make_unique<MockTestItem>(), 3};
        x.Increment();
        CHECK(x.GetQuantity() == 4);
        x.Decrement();
        CHECK(x.GetQuantity() == 3);
    }
    {
        IS x{std::make_unique<MockTestItem>(), 64};
        x.Decrement().Decrement().Decrement();
        CHECK(x.GetQuantity() == 61);
    }
}

TEST_CASE("bool() operator, Contains", "[operator, inventoryslot]") {
    {
        IS x{std::make_unique<MockTestItem>(), 3};
        CHECK(x);
        x.Reset();
        CHECK(!x);
    }
    {
        IS x{std::make_unique<MockTestItem>(), 64};
        CHECK(x.Contains(0));
        CHECK(x.Contains("TestItem"));
        CHECK(!x.Contains("testitem"));
        CHECK(x);
    }
}

TEST_CASE("Insertion and Deletion", "[inventoryslot]") {
    {
        std::string y = "TestItem";
        IS x{std::make_unique<MockTestItem>(), 3};
        CHECK(x.Contains("TestItem"));
        CHECK(x.Contains(y));
        CHECK(x.GetQuantity() == 3);

        x.InsertNew(std::make_unique<MockTestItem>(0, "one"), 2);
        CHECK(x.Contains("one"));
        CHECK(!x.Contains("TestItem"));
        CHECK(x.GetQuantity() == 5);
        x.Increment();
        x.Remove(0);
        CHECK(x.GetQuantity() == 6);
        x.Insert(0);
        auto overflow = x.Insert(44);
        CHECK(x.GetQuantity() == 50);
        CHECK(overflow == 0);
        auto underflow = x.Remove(7);
        CHECK(x.GetQuantity() == 43);
        CHECK(underflow == 0);

        underflow = x.Remove(100);
        CHECK(x.GetQuantity() == 0);
        CHECK(underflow == 57);

        overflow = x.Insert(100);
        CHECK(x.GetQuantity() == Inventory::MAX_ITEMS_PER_SLOT);
        CHECK(Inventory::MAX_ITEMS_PER_SLOT + overflow == 100);
    }
    {
        // this is all that is allowed
        IS x{std::make_unique<MockTestItem2>(), 1};
        auto overflow = x.Insert(100);
        CHECK(overflow == 100);
        CHECK(x.GetQuantity() == 1);
        CHECK(x.IsFull());
        x.Remove(0);
        CHECK(x.GetQuantity() == 1);
        CHECK(x.IsFull());
        x.Remove(1);
        CHECK(x.GetQuantity() == 0);
        CHECK(x.IsEmpty());
        CHECK(!x.IsFull());
    }
    {
        IS x{std::make_unique<MockTestItem2>(), 1};
        auto underflow = x.Remove(100);
        CHECK(underflow == 99);
        CHECK(x.IsEmpty());
        CHECK(x.GetItem() == nullptr);
    }
}

/* **********************************************************
 * INVENTORY SECTION
 * **********************************************************/

TEST_CASE("Swap Slots simple", "[inventory, swap]") {
    // Swapping null item and real item
    Inventory inv;
    CHECK(inv.GetHandSlotIndex() == 0);
    CHECK(inv.GetHand() == nullptr);
    inv.AddItem(std::make_unique<MockTestItem>(0, "TestItem"), 3);

    inv.SwapSlots(0, Inventory::HOTBAR_SIZE);

    CHECK(inv.GetHandSlotIndex() == 0);
    CHECK(inv.GetHand()->GetName() == "TestItem");
}

TEST_CASE("Inventory Add and Remove", "[add, remove, inventory]") {
    Inventory inv;
    auto& invView = inv.GetInventoryArray();
    MockWorldBase world;
    inv.AddItem(std::make_unique<MockTestItem>(0, "TestItem"), 500);
    inv.AddItem(std::make_unique<RealItem>(1, world), 3);
    inv.AddItem(std::make_unique<RealUItem>(4, 4, world));
    inv.SwapSlots(Inventory::HOTBAR_SIZE, 2);

    inv.RemoveItem("TestItem", 5);
    CHECK(invView.at(Inventory::HOTBAR_SIZE + 1).GetQuantity() == Inventory::MAX_ITEMS_PER_SLOT - 5);
    inv.RemoveUniqueItem(4);
    auto result = inv.GetTotal("RealUItem");
    CHECK(result == 0);

    inv.ClearInventory();
    inv.AddItem(std::make_unique<MockTestItem>(0, "TestItem"), 3);
    CHECK(inv.GetTotal("TestItem") == 3);
    inv.RemoveItem("TestItem", 5);
    result = inv.GetTotal("TestItem");
    CHECK(result == 0);
    inv.AddItem(std::make_unique<MockTestItem>(0, "TestItem"), 3);
    result = inv.GetTotal("TestItem");
    CHECK(result == 3);
}

TEST_CASE("Inventory RemoveItem all-or-nothing flag behavior", "[inventory, remove, all-or-nothing]") {
    Inventory inv;
    inv.AddItem(std::make_unique<MockTestItem>(0, "TestItem"), 15);

    SECTION("True flag leaves inventory unchanged when amount is less than total") {
        const auto remaining = inv.RemoveItem("TestItem", 12, true);
        CHECK(remaining == 12); // amount tried to remove is returned
        CHECK(inv.GetTotal("TestItem") == 15);
    }

    SECTION("False flag removes what is available") {
        const auto remaining = inv.RemoveItem("TestItem", 12, false);
        CHECK(remaining == 0);
        CHECK(inv.GetTotal("TestItem") == 3);
    }

    SECTION("Check again with different number") {
        const auto remaining = inv.RemoveItem("TestItem", 10, true);
        CHECK(remaining == 10);
        CHECK(inv.GetTotal("TestItem") == 15);
    }
}

TEST_CASE("Inventory Basic Example walkthrough -- one item", "[inventory]") {
    // One item
    Inventory inv;
    CHECK(inv.GetHandSlotIndex() == 0);
    CHECK(inv.GetHand() == nullptr);
    inv.AddItem(std::make_unique<MockTestItem>(0, "TestItem"), 3);
    CHECK(inv.GetHandSlotIndex() == 0);
    CHECK(inv.GetHand() == nullptr);
    inv.SwapSlots(Inventory::HOTBAR_SIZE, 0);
    CHECK(inv.GetHandSlotIndex() == 0);
    CHECK(inv.GetHand()->GetName() == "TestItem");
    CHECK(inv.GetHand()->GetId() == 0);
    CHECK(inv.GetHandQuantity() == 3);
    auto& invView = inv.GetInventoryArray();
    CHECK(!invView.at(Inventory::HOTBAR_SIZE).GetItem());
    CHECK(invView.at(Inventory::HOTBAR_SIZE).GetQuantity() == 0);
}

TEST_CASE("Inventory Unique Item vs Non-Unique -- MultiItem", "[inventory, item, constructor]") {
    // !NOTE!
    // If item is UNIQUE then ID MATTERS
    // otherwise it just doesn't.
    // Also check the ways to do "additem"
    Inventory inv;
    MockWorldBase world;
    auto ov1 = inv.AddItem(std::make_unique<RealItem>(0, world), 3);
    auto ov2 = inv.AddItem(std::make_unique<RealUItem>(1, 10, world), 1);
    auto ov3 = inv.AddItem(std::make_unique<RealItem>(2, world), 3);
    auto ov4 = inv.AddItem(std::make_unique<RealItem>(55, world), 5);
    auto ov5 = inv.AddItem(std::make_unique<RealItem>(0, world), 5);

    auto total = inv.GetTotal("RealItem");
    CHECK(ov1 == 0);
    CHECK(ov2 == 0);
    CHECK(ov3 == 0);
    CHECK(ov4 == 0);
    CHECK(ov5 == 0);
    CHECK(total == 16);
    auto& invView = inv.GetInventoryArray();
    CHECK(invView.at(Inventory::HOTBAR_SIZE).GetQuantity() == 16);
    CHECK(invView.at(Inventory::HOTBAR_SIZE + 1).GetQuantity() == 1);

    auto overflow = inv.AddItem<RealItem>(200, 0, world);

    CHECK(invView.at(Inventory::HOTBAR_SIZE).GetQuantity() == Inventory::MAX_ITEMS_PER_SLOT);
    CHECK(overflow == 0);
    CHECK(invView.at(Inventory::HOTBAR_SIZE + 2).GetQuantity() == Inventory::MAX_ITEMS_PER_SLOT);
    CHECK(invView.at(Inventory::HOTBAR_SIZE + 1).GetQuantity() == 1);

    // now clear and restart with mixture of addItem calls
    inv.ClearInventory();
    for (auto& slot: invView) {
        CHECK(!slot.GetItem());
    }
    // now create more slots
    inv.AddItem<RealItem>(5, 0, world);
    inv.AddItem(std::make_unique<RealItem>(1, world), 5);
    inv.AddItem(std::make_unique<RealItem>(0, world), 5);
    inv.AddItem(std::make_unique<RealItem>(55, world), 5);
    // Quantity first then (id, gold, world)
    inv.AddItem<RealUItem>(1, 21, 4, world);
    inv.AddItem(std::make_unique<RealUItem>(22, 0, world));
    CHECK(invView.at(Inventory::HOTBAR_SIZE + 1).GetItem()->GetId() == 21);
    CHECK(invView.at(Inventory::HOTBAR_SIZE + 2).GetItem()->GetId() == 22);
    auto qu = inv.GetTotal("RealItem");
    CHECK(qu == 20);
    CHECK(invView.at(Inventory::HOTBAR_SIZE + 3).IsEmpty());
}

TEST_CASE("Inventory Other methods", "[inventory, methods]") {
    Inventory inv;
    inv.AddItem(std::make_unique<MockTestItem>(0, "TestItem"), 3);
    inv.AddItem(std::make_unique<MockTestItem>(1, "TestItem"), 3);
    inv.AddItem(std::make_unique<MockTestItem>(4, "TestItem"), 3);

    inv.ClearInventory();
    auto& invView = inv.GetInventoryArray();
    for (auto& slot: invView) {
        CHECK(!slot.GetItem());
        CHECK(slot.GetQuantity() == 0);
    }
    CHECK(inv.GetHand() == nullptr);
    CHECK(inv.GetHandSlotIndex() == 0);
    inv.HotBarIndexMove(4);
    CHECK(inv.GetHandSlotIndex() == 4);
    inv.HotBarIndexMove(Inventory::HOTBAR_SIZE + 3);
    CHECK(inv.GetHandSlotIndex() == 3);

    inv.HotBarIndexInc();
    CHECK(inv.GetHandSlotIndex() == 5);
    inv.HotBarIndexDec();
    CHECK(inv.GetHandSlotIndex() == 4);
}

TEST_CASE("Check Asserts, some edge cases", "[none]") {
    Inventory inv;
    [[maybe_unused]] MockWorldBase world;
    // some edge cases:
    auto result = inv.RemoveItem("none", 3);
    CHECK(result == 3);

    auto remaining = inv.AddItem(nullptr, 5);
    CHECK(remaining == 5);

    // Should hit assert:
    auto item = std::make_unique<RealUItem>(1, 5, world);
    CHECK(item->IsUnique());

    // inv.AddItem(std::move(item), 5);
}
