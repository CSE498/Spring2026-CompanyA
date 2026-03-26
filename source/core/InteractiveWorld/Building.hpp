/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Represents an upgradable building in the interactive game world.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>
#include <expected>
#include <optional>
#include <string>
#include <vector>

namespace cse498 {
// TODO: This is temporary. Figure out what items will be and how to structure
// them in code
enum class ItemType { Wood, Stone, Metal };

/// @class Building
/// @brief Upgradable building. Starts at level 0.
class Building {
public:
  // The quantity and type of items needed for an upgrade
  struct BuildingUpgrade {
    ItemType item; // What item the upgrade requires
    int quantity;  // The quantity of items needed to upgrade
  };

  // Reason for rejecting an upgrade
  enum class UpgradeRejectionType {
    AlreadyMaxLevel,
    IncorrectItemType,
    NotEnoughItems
  };

private:
  std::string m_name{};        // Name of the building
  int m_level{};               // The current level of the building
  float m_rateModifier = 0.25; // Percent increase as decimal ex: 0.25->25%
  std::vector<BuildingUpgrade> m_upgrades{}; // The upgrade cost per level

  /**
   * Get a reference to the next upgrade BuildingUpgrade struct
   * @return BuildingUpgrade struct for the next upgrade (holds type of items
   * and quantity needed)
   */
  const BuildingUpgrade &GetNextUpgrade() const {
    assert(m_level < GetMaxLevel() && "Building already max level!");
    return m_upgrades[m_level];
  }

  /**
   * Checks if an upgrade is able to be applied to the building.
   * @param itemType The type of item for for the upgrade
   * @param quantity The quantity of items the player has available
   * @return void if the upgrade is successful, UpgradeRejectionType describing
   * why it failed
   */
  std::expected<void, UpgradeRejectionType>
  IsUpgradeValid(const ItemType &itemType, const int &quantity) const {
    if (m_level >= GetMaxLevel())
      return std::unexpected(UpgradeRejectionType::AlreadyMaxLevel);

    const auto &cost = GetNextUpgrade();

    if (itemType != cost.item)
      return std::unexpected(UpgradeRejectionType::IncorrectItemType);

    if (quantity < cost.quantity)
      return std::unexpected(UpgradeRejectionType::NotEnoughItems);

    return {};
  }

public:
  Building() = delete;
  /**
   * Constructor
   * @param name name of the building
   */
  Building(std::string name) : m_name{name} {};
  /**
   * Get Max level for this building
   * @return max level as an int
   */
  int GetMaxLevel() const { return m_upgrades.size(); }
  /**
   * Get current building level
   * @return current level as int
   */
  int GetCurrentLevel() const { return m_level; }
  /**
   * Get Next Upgrade level
   * @return
   */
  int GetNextUpgradeLevel() const {
    if (m_level == GetMaxLevel())
      return m_level;
    return m_level + 1;
  }
  /**
   * Check if the current level is the max level
   * @return bool if the building is max level or not
   */
  bool IsMaxLevel() const { return m_level >= GetMaxLevel(); }
  /**
   * Get the quantity of resources needed for the next upgrade
   * @return Quantity of resources as int
   */
  int GetNextUpgradeQuantity() const { return GetNextUpgrade().quantity; }
  /**
   * Get the type of resource needed for the next upgrade
   * @return type of resource needed as ItemType (type TBD)
   */
  ItemType GetNextUpgradeType() const { return GetNextUpgrade().item; }
  /**
   * Set the name of the building
   * @param newName new name of the building
   */
  void SetName(std::string newName) { m_name = newName; }
  /**
   * Set the rate modifier
   * @param rate rate to set to
   */
  void SetRateModifier(float rate) { m_rateModifier = rate; }
  /**
   * Get the rate modifier
   * @return rate modifier
   */
  float GetRateModifier() { return m_rateModifier; }
  /**
   * Get the name of the building
   * @return name of the building
   */
  std::string GetName() const { return m_name; }
  /**
   * Add an upgrade level to the building
   * @param item type of item needed for the upgrade
   * @param quantity number of items needed for the upgrade
   */
  void AddUpgrade(ItemType item, int quantity) {
    assert(quantity > 0 && "Upgrade cost must be positive!");
    m_upgrades.push_back({item, quantity});
  }
  /**
   * Upgrade the building
   * @param itemType Type of item to use
   * @param quantity Number of items in the players inventory.
   * @return void if the upgrade is successful, UpgradeRejectionType if not
   * successful
   */
  std::expected<void, UpgradeRejectionType> Upgrade(const ItemType &itemType,
                                                    int &quantity) {
    auto result = IsUpgradeValid(itemType, quantity);

    if (!result)
      return std::unexpected(result.error());

    // TODO: Have the player inventory change itself, not the building change it
    // const auto& cost = GetNextUpgrade();
    // quantity -= cost.quantity;
    m_level++;
    return {};
  }

  /**
   * Get all BuildingUpgrade structs for the building. For UI.
   * @return vector of all of the Upgrades
   */
  const std::vector<BuildingUpgrade> &GetAllUpgrades() const {
    return m_upgrades;
  }
  /**
   * Get the BuildingUpgrade struct for the next upgrade. For UI.
   * @return the next BuildingUpgrade struct if it exists
   */
  std::optional<BuildingUpgrade> GetNextUpgradeInfo() const {
    if (m_level >= GetMaxLevel())
      return std::nullopt;
    return m_upgrades[m_level];
  }
};
} // namespace cse498
