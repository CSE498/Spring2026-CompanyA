/**
 * @file Inventory.hpp
 * @author Logan Rimarcik
 *
 * The inventory of the main player character
 *
 * NOTE: 2 types of items:
 * Unique items max stack size == 1, size_t id identifies them as different objs
 * Non-Unique items max stack size is MAX_STACK_SIZE, size_t id is IGNORED, std::string name
 * identifies them as different objects otherwise they stack.
 * ^^ if you want two variants of the same item with the same name, then add a std::string id as well
 * I was unable to push hard enough to convince everyone for this change.
 */

#pragma once

#include "core/ItemBase.hpp"
#include <optional>
#include <array>
#include <unordered_map>
#include <set>
#include <iostream>
#include <utility>
#include <memory>

namespace cse498
{
/**
 * This is a temporary placeholding struct that will be deleted once dependancy of Item class is met
 * NOTE!!!
 * NOTE!!!
 * This is NOT my class. This is a placeholder from Group 15 that I needed to implement inventory
 * I wish an early pull request was done so we could both access this, but this class is weird.
 */
class Item // I am not responsible for tests!!
{
private:
    const size_t mId;              // An id unique to this item
    std::string mName;             // Name of the item
    std::string mImagePath;       // Filepath to the image for this item
    int mGold;                     // Amount of gold the item can be sold/bought for
    [[maybe_unused]] const WorldBase & mWorld;        // World this item is in
    // Put something here for specific location? Whether or not it is in player inventory?

protected:
    /// Allows certain items to set this attribute or make a setter
    /// This attribute determines stack size default or stack size 1 and mId identifies it
    bool mUnique = false;


public:
    Item(size_t id, std::string  name, std::string  image_path, int gold,
        const WorldBase & world): mId(id), mName(std::move(name)), mImagePath(std::move(image_path)),
        mGold(gold), mWorld(world) {}

    [[nodiscard]] size_t GetId() const {return mId;}
    [[nodiscard]] std::string GetName() const  {return mName;}
    [[nodiscard]] std::string GetImagePath() const {return mImagePath;}
    [[nodiscard]] int GetGold() const  {return mGold;}
    [[nodiscard]] bool IsUnique() const {return mUnique;}

    void SetName(const std::string & name) {mName = name;}
    void SetImagePath(const std::string & image_path) {mImagePath = image_path;}
    void SetGold(int gold) {mGold = gold;}

    [[nodiscard]] virtual bool IsWeapon() const {return false;}
    [[nodiscard]] virtual bool IsConsumable() const {return false;}
    /**
     * An item is attempted to be used -- this defines what it does upon use
     * default is that it doesn't do anything
     */
    virtual void Use() {}

    virtual void DestroyItem() {
        // TODO: Delete the item from the world
    };
    virtual ~Item() = default;
};




/**
 * !IMPORTANT
 * Hotbar leftmost slot = slot 0 then slot HOTBAR_SIZE + 1 is top left of backpack
 */
class Inventory
{
public:
    static constexpr size_t BACKPACK_SIZE = 20;
    static constexpr size_t HOTBAR_SIZE = 10;
    static constexpr size_t INVENTORY_SIZE = BACKPACK_SIZE + HOTBAR_SIZE;
    static constexpr size_t ITEMS_PER_ROW = 5; // 5 * 4 grid + 10 in hotbar? idc if changed
    static constexpr size_t MAX_ITEMS_PER_SLOT = 64;

    /**
     * Used to add items to the inventory placed in the backpack first and foremost
     * Clean hotbars are nice in games.
     * @param item - item to add --
     * @return size_t = amount remaining to be added. 0 == everything was added
     */
    size_t AddItem(std::unique_ptr<Item>, size_t quantity = 1);

    /**
     * Overload so you don't have to build the item then pass it in.
     * @tparam ITEM_T - needs to have default constructor and be derived from Item
     * @param quantity - the amount of the item to add
     * @return
     */
    template<typename ITEM_T, typename... Args>
    requires std::derived_from<ITEM_T, Item>
    size_t AddItem(size_t quantity = 1, Args&&... args) // it may be better to replace forwarding to 2 args: id, world
    {
        return AddItem(std::make_unique<ITEM_T>(std::forward<Args>(args)...), quantity);
    }

    /**
     * Removes the item if it is NOT UNIQUE. NAME ONLY MATCHING
     * if a unique item --> remove 7 then removes the first 7 with matching names
     * @param name item category
     * @param amount - amount of the item to remove.
     * @param isAllOrNothing - True ==> removes all or removes nothing. False then removes what is available
     * True ==> return amount if nothing removed , 0 everything removed
     * False ==> return amount removed. Requested 5 removal, had 2 --> return 2
     * @return ^^^
     */
    size_t RemoveItem(const std::string& name, size_t amount = 1, bool isAllOrNothing = true);

    /**
     * Removes a particular item that was created. This is for unique items of stack size 1
     * @param itemId item_id which is specific to one particular item
     * @return amount remaining for consistency
     * 1 ==> FAILURE 1 item remaining in slot
     * 0 ==> Success all items removed.
     */
    size_t RemoveUniqueItem(const size_t itemId);

    /**
     * Trading-like swapping two items.
     * @param item_id1
     * @param item_id2
     */
    void SwapSlots(size_t slotIndex1, size_t slotIndex2);
    void ClearInventory();
    void HotBarIndexMove(int index) { mCurrentHotbarSlot = index; };
    void HotBarIndexInc() { mCurrentHotbarSlot = (mCurrentHotbarSlot + 1) % HOTBAR_SIZE; }
    void HotBarIndexDec() { mCurrentHotbarSlot = (mCurrentHotbarSlot - 1 + HOTBAR_SIZE) % HOTBAR_SIZE; }
    size_t GetTotal(const std::string& name) const;

    /**
     * Finds the first item instance with the given name.
     *
     * @param name Item category/name to search for
     * @return Pointer to an item instance if found, otherwise nullptr
     */
    [[nodiscard]] const Item* FindFirstItem(const std::string& name) const;

    /**
     * Hand slot item. Quantity is other method
     * @return Gives you the item in the hand slot.
     */
    Item* GetHand() const { return mInventory.at(mCurrentHotbarSlot).GetItem(); }
    size_t GetHandQuantity() const { return mInventory.at(mCurrentHotbarSlot).GetQuantity(); }
    [[nodiscard]] size_t GetHandSlotIndex() const { return mCurrentHotbarSlot; }

    /**
     * This object actually can become quite complex, and I don't want
     * meddling with internal variables since it can break things
     * This class SHOULD NOT BE USED OUTSIDE THIS CLASS but is public for testing
     */
    class InventorySlot
    {
    private:
        std::unique_ptr<Item> mItem = nullptr;
        size_t mQuantity = 0;
    public:
        InventorySlot() = default;
        /**
         * if the item IS UNIQUE then quantity NEEDS TO BE 1. Otherwise, not unique ==> quantity in [1, MAX_ITEMS_PER_SLOT]
         * @param item item to put in the inventory slot
         * @param quantity - quantity of the item
         */
        InventorySlot(std::unique_ptr<Item> item, size_t quantity) : mItem(std::move(item)), mQuantity(quantity)
        {
            // I would've used exceptions here otherwise.
            assert(quantity > 0 && quantity <= MAX_ITEMS_PER_SLOT);
            // This should never occur. An item that is unique shouldn't have quantity > 1 this would be strange
            if (mItem)
                assert(!mItem->IsUnique() || quantity == 1);
        }
        void Reset() { mItem.reset(); mQuantity = 0; }

        [[nodiscard]] bool IsEmpty() const {return !mItem;}
        [[nodiscard]] bool IsFull() const {return (mQuantity >= Inventory::MAX_ITEMS_PER_SLOT || (mItem && mItem->IsUnique()));}
        [[nodiscard]] size_t GetQuantity() const {return mQuantity;}
        [[nodiscard]] bool IsRoom(size_t amount) const { return (mQuantity + amount) <= MAX_ITEMS_PER_SLOT; }

        InventorySlot& Decrement()
        {
            mQuantity--;
            if (mQuantity == 0) mItem = nullptr;
            return *this;
        }
        InventorySlot& Increment()
        {
            mQuantity++;
            return *this;
        }
        explicit operator bool() const {return static_cast<bool>(mItem);}
        [[nodiscard]] bool Contains(const std::string& name) { return (mItem && mItem->GetName() == name); }
        [[nodiscard]] bool Contains(const size_t itemId) { return (mItem && mItem->GetId() == itemId); }
        /**
         * Inserts and overrides the slot with specified quantity
         * @param item some defined item
         * @param quantity some defined quantity of item
         */
        size_t InsertNew(std::unique_ptr<Item> item, size_t quantity)
        {
            if (item && item->IsUnique())
            {
                if (quantity == 1)
                {
                    mQuantity = 1;
                    mItem = std::move(item);
                    return 0;
                }
                return quantity;
            }
            mItem = std::move(item);
            return Insert(quantity);
        }
        size_t Insert(size_t quantity)
        {
            if (mItem && mItem->IsUnique())
                return quantity;
            mQuantity += quantity;
            if (mQuantity > MAX_ITEMS_PER_SLOT)
            {
                auto overflow = mQuantity - MAX_ITEMS_PER_SLOT;
                mQuantity = MAX_ITEMS_PER_SLOT;
                return overflow;
            }
            return 0;
        }
        size_t Remove(size_t quantity)
        {
            if (mQuantity <= quantity)
            {
                mItem.reset();
                auto underflow = (quantity - mQuantity);
                mQuantity = 0;
                return underflow;
            }
            mQuantity -= quantity;
            return 0;
        }
        [[nodiscard]] Item* GetItem() const
        {
            return mItem.get();
        }

    };

    /**
     * Exposure of a const reference to the inventory. Read-only access to see states
     * @return const ref of inventory.
     */
    const std::array<InventorySlot, INVENTORY_SIZE>& GetInventoryArray() const { return mInventory; }
private:
    /**
     * Enforces that removal will take place in the backpack first then
     * the hotbar
     */
    struct CircularCompare
    {
        size_t Transform(size_t x) const
        {
            if (x >= HOTBAR_SIZE) return (x - HOTBAR_SIZE);
            return x + (INVENTORY_SIZE - HOTBAR_SIZE + 1);
        }
        bool operator()(size_t a, size_t b) const
        {
            return Transform(a) < Transform(b);
        }
    };

    // I don't care what the size of the inventory is or if we change the structure.
    /// This size includes the hotbar I would presume
    /// Optional: if the item doesn't exist then it is an empty item.
    std::array<InventorySlot, INVENTORY_SIZE> mInventory{};

    /// This is for constant speed lookup -- a little overkill but nice
    /// key: size_t item_id --> Value: index locations in array
    /// Ordered for consistency in removal
    std::unordered_map<std::string, std::set<size_t, CircularCompare>> mItemMap;

    /// this is the selected hotbar slot
    size_t mCurrentHotbarSlot = 0;


};

std::ostream& operator<<(std::ostream& os, const Inventory::InventorySlot& slot);
/**
 * Shows example layout of the inventory. Similar operation should be done in practice
 * @param os - output stream
 * @param inv - inventory
 * @return ostream
 */
std::ostream& operator<<(std::ostream& os, const Inventory& inv);


}


