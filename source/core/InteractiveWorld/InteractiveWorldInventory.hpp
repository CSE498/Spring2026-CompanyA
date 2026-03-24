#pragma once

#include <map>
#include <cassert>
#include "Building.hpp"


namespace cse498 {

    class InteractiveWorldInventory {
    private:
        std::map<ItemType, int> m_items;

    public:
        // Add resources of one type
        void AddItem(ItemType item, int amount) {
            assert(amount >= 0 && "AddItem amount must not be negative");
            m_items[item] += amount;
        }

        // Remove resources if enough exist
        // Returns true if successful, false if not enough
        bool RemoveItem(ItemType item, int amount) {
            assert(amount >= 0 && "RemoveItem amount must not be negative");

            if (!HasEnough(item, amount)) {
                return false;
            }

            m_items[item] -= amount;
            return true;
        }

        // Check how many of one resource exist
        int GetAmount(ItemType item) const {
            auto it = m_items.find(item);
            if (it == m_items.end()) {
                return 0;
            }
            return it->second;
        }

        // Check if inventory has enough of a resource
        bool HasEnough(ItemType item, int amount) const {
            assert(amount >= 0 && "HasEnough amount must not be negative");
            return GetAmount(item) >= amount;
        }

        // Remove all stored resources
        void Clear() {
            m_items.clear();
        }

        // Check if inventory has no resources
        bool Empty() const {
            return m_items.empty();
        }
    };

} // namespace cse498