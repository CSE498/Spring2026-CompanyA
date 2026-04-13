#pragma once

#include "ItemType.hpp"

#include <cassert>
#include <cstddef>
#include <map>

namespace cse498 {

/**
 * @class InteractiveWorldInventory
 * @brief Stores and manages world resources such as wood, stone, and metal.
 *
 * This class keeps track of resource totals by ItemType. It supports adding
 * resources, removing resources, checking whether enough resources exist for
 * an upgrade, clearing all stored resources, and exposing the stored data for
 * UI or save/load systems.
 *
 * @note This is currently a resource-total inventory, not a slot-based
 * inventory.
 */
class InteractiveWorldInventory {
public:
  using ItemCount = std::size_t;

  /// Maximum number of items allowed for one resource type.
  static constexpr ItemCount MAX_ITEMS_PER_TYPE = 999;

private:
  std::map<ItemType, ItemCount> m_items;

public:
  /**
   * @brief Add resources of one type to the inventory.
   * @param item The resource type to add.
   * @param amount The quantity to add.
   * @return true if the add succeeded, false if it would exceed the maximum.
   */
  bool AddItem(ItemType item, std::size_t amount) {
    if (amount == 0) {
      return true;
    }

    const ItemCount current_amount = GetAmount(item);

    if (current_amount + amount > MAX_ITEMS_PER_TYPE) {
      return false;
    }

    m_items[item] += amount;
    return true;
  }

  /**
   * @brief Remove resources of one type from the inventory.
   * @param item The resource type to remove.
   * @param amount The quantity to remove.
   * @return true if removal succeeded, false if there were not enough items.
   */
  bool RemoveItem(ItemType item, ItemCount amount) {
    if (!HasEnough(item, amount)) {
      return false;
    }

    m_items[item] -= amount;

    if (m_items[item] == 0) {
      m_items.erase(item);
    }

    return true;
  }

  /**
   * @brief Get the quantity currently stored for a resource type.
   * @param item The resource type to query.
   * @return The stored quantity, or 0 if the item type is not present.
   */
  [[nodiscard]] ItemCount GetAmount(ItemType item) const {
    if (!m_items.contains(item)) {
      return 0;
    }
    return m_items.at(item);
  }

  /**
   * @brief Check whether the inventory contains at least a given amount.
   * @param item The resource type to query.
   * @param amount The quantity required.
   * @return true if enough of the resource exists, false otherwise.
   */
  [[nodiscard]] bool HasEnough(ItemType item, ItemCount amount) const {
    return GetAmount(item) >= amount;
  }

  /**
   * @brief Get a read-only view of all stored inventory data.
   * @return A const reference to the internal item map.
   */
  [[nodiscard]] const std::map<ItemType, ItemCount> &GetItems() const {
    return m_items;
  }

  /**
   * @brief Remove all stored resources from the inventory.
   */
  void Clear() { m_items.clear(); }

  /**
   * @brief Check whether the inventory is empty.
   * @return true if no resources are stored, false otherwise.
   */
  [[nodiscard]] bool Empty() const { return m_items.empty(); }
};

} // namespace cse498
