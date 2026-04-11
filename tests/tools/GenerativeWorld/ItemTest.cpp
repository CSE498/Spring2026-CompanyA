/**
 * @file ItemTest.cpp
 * @author Group 2
 */

#include "../../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../../source/core/item/Item.hpp"


namespace cse498
{
class TestItemWorld : public WorldBase
{
public:
    int DoAction([[maybe_unused]] AgentBase& agent, [[maybe_unused]] size_t action_id) override { return 0; }
};

class TestUniqueItem : public Item
{
public:
    TestUniqueItem(size_t id, const WorldBase& world)
        : Item(id, "Unique", "unique.png", 11, world)
    {
    }

    [[nodiscard]] bool IsUnique() const override { return true; }
};

class TestItemConsumable : public Item
{
public:
    TestItemConsumable(size_t id, const WorldBase& world)
        : Item(id, "DualType", "dual.png", 25, world)
    {
    }

    [[nodiscard]] bool IsWeapon() const override { return true; }
    [[nodiscard]] bool IsConsumable() const override { return true; }
    void Use() override { m_used = true; }
    [[nodiscard]] bool WasUsed() const { return m_used; }

private:
    bool m_used = false;
};
} // namespace cse498

using cse498::Item;
using cse498::TestItemWorld;
using cse498::TestUniqueItem;
using cse498::TestItemConsumable;

TEST_CASE("Item base constructor, accessors, and mutators", "[core, item]")
{
    TestItemWorld world;
    Item item(42, "Potion", "assets/potion.png", 15, world);

    CHECK(item.GetId() == 42);
    CHECK(item.GetName() == "Potion");
    CHECK(item.GetImagePath() == "assets/potion.png");
    CHECK(item.GetGold() == 15);
    CHECK(item.IsUnique() == false);
    CHECK(item.IsWeapon() == false);
    CHECK(item.IsConsumable() == false);

    item.SetName("SuperPotion");
    item.SetImagePath("assets/super_potion.png");
    item.SetGold(40);
    item.Use();

    CHECK(item.GetName() == "SuperPotion");
    CHECK(item.GetImagePath() == "assets/super_potion.png");
    CHECK(item.GetGold() == 40);
}

TEST_CASE("Item derived behavior can customize uniqueness and virtuals", "[core, item]")
{
    TestItemWorld world;

    TestUniqueItem uniqueItem(100, world);
    CHECK(uniqueItem.IsUnique() == true);
    CHECK(uniqueItem.IsWeapon() == false);
    CHECK(uniqueItem.IsConsumable() == false);

    TestItemConsumable consumableItem(101, world);
    CHECK(consumableItem.IsWeapon() == true);
    CHECK(consumableItem.IsConsumable() == true);
    CHECK(consumableItem.WasUsed() == false);
    consumableItem.Use();
    CHECK(consumableItem.WasUsed() == true);
}

