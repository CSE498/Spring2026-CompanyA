/**
 * @file Inventory.cpp
 * @author Logan Rimarcik
 */

#include "Inventory.hpp"

#include <numeric>
#include <ranges>

namespace cse498
{
size_t Inventory::AddItem(std::unique_ptr<Item> item, size_t quantity)
{
    if (!item)
        return quantity;

    // If this fails you tried to add multiple unique items. :
    // multiple unique items need multiple unique ids which isn't possible here.
    assert(!item->IsUnique() || quantity == 1);
    // First see if there is a slot that already contains the item
    // Given this isn't a unique item
    if (!item->IsUnique() && mItemMap.contains(item->GetName()))
    {
        // Cool, it already exists in the inventory.
        for (const auto& slotIndex : mItemMap.at(item->GetName()))
        {
            // Check each spot for the first spot NOT FULL
            if (!mInventory.at(slotIndex).IsFull())
            {
                quantity = mInventory.at(slotIndex).Insert(quantity);
                if (quantity == 0)
                    return 0;
            }
        }
    }
    // either there is no item or the spot is full so we don't care about that slot
    // find the next slot available (empty):

    // Unique items can only be inserted in new empty slots.
    for (size_t i = 0; i < INVENTORY_SIZE; ++i)
    {
        size_t index = (i + HOTBAR_SIZE) % INVENTORY_SIZE;
        // if slot is empty
        if (!mInventory.at(index))
        {
            const auto name = item->GetName();
            quantity = mInventory.at(index).InsertNew(std::move(item), quantity);
            // when [] is actually useful to insert if it doesn't exist
            mItemMap[name].insert(index);
            if (quantity == 0)
                return 0;
        }
    }
    return quantity;
}
size_t Inventory::RemoveItem(const std::string& name, size_t amount, bool isAllOrNothing)
{
    // inventory contains the item. Remove it
    auto itr = mItemMap.find(name);
    if (itr == mItemMap.end())
        return amount;

    size_t total = GetTotal(name);
    if (isAllOrNothing && amount > total)
        return amount;

    // now the main loop:
    auto& itemIndices = itr->second;
    assert(!itemIndices.empty()); // if this fails something is wrong
    // size_t anyIndex = *(itemIndices.begin());
    // Note if this fails, you tried to remove a unique item using its name.
    // This is NOT ACCEPTABLE, doing this means it removes an arbitrary item of that name and not a specified unique item
    // This is NOT INPUT VALIDATION since inputting this is fine and works in this function I just don't want it to work.
    // idk actually... maybe it is ok..
    // assert(mInventory.at(anyIndex).GetItem().has_value() && !mInventory.at(anyIndex).GetItem().value().IsUnique());

    for (auto iterator = itemIndices.begin(); iterator != itemIndices.end(); )
    {
        size_t index = *iterator;
        amount = mInventory.at(index).Remove(amount);
        // NOTE both of these increment. This isn't UB but could shoot myself in the foot
        if (mInventory.at(index).IsEmpty())
        {
            iterator = itemIndices.erase(iterator);
        }
        else
        {
            ++iterator;
        }
        if (amount == 0)
            break;
    }
    if (itemIndices.empty())
        mItemMap.erase(itr);

    return amount;

}
size_t Inventory::GetTotal(const std::string &name) const
{
    if (mItemMap.contains(name))
    {
        auto indices = mItemMap.at(name);
        // I swear they rather have this than a for loop, but I had to fix this line TWICE! This is verbose.
        size_t total = std::accumulate(indices.begin(), indices.end(), size_t{0},
            [this](size_t sum, size_t index) { return sum + mInventory.at(index).GetQuantity(); });
        return total;
    }
    return 0;
}

size_t Inventory::RemoveUniqueItem(const size_t itemId)
{
    // We don't have a lookup for itemId so we straight up search for it
    for (size_t i = 0; i < INVENTORY_SIZE; ++i)
    {
        if (mInventory.at(i).Contains(itemId))
        {
            mInventory.at(i).Reset();
            return 0;
        }
    }
    return 1;
}


void Inventory::SwapSlots(size_t slotIndex1, size_t slotIndex2)
{
    std::swap(mInventory.at(slotIndex1), mInventory.at(slotIndex2));
}
void Inventory::ClearInventory()
{
    mItemMap.clear();
    for (auto& slot : mInventory)
    {
        slot.Reset();
    }
}
std::ostream& operator<<(std::ostream& os, const Inventory::InventorySlot& slot)
{
    // just for some visualization to check state.
    if (!slot.GetItem())
    {
        os << "empty";
        return os;
    }
    // else:

    os << "(I:" << slot.GetItem()->GetName()
       << ", Q:" << slot.GetQuantity() << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Inventory& inv)
{
    os << "Inventory: \n";
    os << "HOTBAR: ";
    for (size_t i = 0; i < Inventory::HOTBAR_SIZE; i++)
    {
        auto& slot = inv.GetInventory().at(i);
        os << slot << " ";
    }
    for (size_t row = 0; row < Inventory::BACKPACK_SIZE / Inventory::ITEMS_PER_ROW; row++)
    {
        os << "\n";
        os << "Row " << row+1 << ":";
        for (size_t col = 0; col < Inventory::ITEMS_PER_ROW; col++)
        {
            auto& slot = inv.GetInventory().at(Inventory::HOTBAR_SIZE + row * Inventory::ITEMS_PER_ROW + col);
            os << slot << " ";
        }
    }
    return os;
}

}
