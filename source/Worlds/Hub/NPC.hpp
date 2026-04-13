/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Represents an NPC that manages interaction with a building.
 * @note Status: PROPOSAL
 **/

/// used codex for doxygen comments, and code structure (checking for UML
/// correctness)

#pragma once

#include "../../core/WorldGrid.hpp"
#include "Building.hpp"
#include "InteractiveWorldInventory.hpp"

#include <cassert>
#include <expected>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace cse498 {

/// @class NPC
/// @brief An interactive NPC that presents and applies building upgrades.
class NPC {
private:
  std::shared_ptr<Building>
      m_building{}; ///< Building associated with this NPC.

  WorldPosition m_position{}; // Position of NPC

  char m_symbol = 'N'; // NPC symbol

public:
  NPC() = delete;

  /**
   * Constructor
   * @param building Building this NPC will interact with
   */
  explicit NPC(std::shared_ptr<Building> building)
      : m_building(std::move(building)) {
    assert(m_building && "NPC must reference a building");
  }

  /**
   * Trigger NPC interaction for the associated building.
   */
  void Interact() const { ShowUpgradeUI(); }

  /**
   * Set world position
   * @param pos new world position
   */
  void SetPosition(WorldPosition pos) { m_position = pos; }
  /**
   * Get world position
   * @return world position
   */
  WorldPosition GetPosition() const { return m_position; }
  /**
   * Set symbol
   * @param s new symbol
   */
  void SetSymbol(char s) { m_symbol = s; }
  /**
   * Get Symbol
   * @return symbol
   */
  char GetSymbol() const { return m_symbol; }

  /**
   * Build the building's upgrade information for a UI layer to consume.
   */
  std::string GetUpgradeUI() const {
    std::ostringstream output;
    output << m_building->GetName()
           << " - level: " << m_building->GetCurrentLevel();

    if (const auto next_upgrade = m_building->GetNextUpgradeInfo();
        next_upgrade.has_value()) {
      output << " | next upgrade: " << next_upgrade->quantity << ' '
             << ItemTypeToString(next_upgrade->item) << " for level "
             << m_building->GetNextUpgradeLevel();
    } else {
      output << " | max level reached";
    }

    return output.str();
  }

  /**
   * Show the building's upgrade information.
   * @param output Stream to write the upgrade information to
   */
  void ShowUpgradeUI(std::ostream &output = std::cout) const {
    output << GetUpgradeUI() << '\n';
  }

  /**
   * Attempt to upgrade the associated building using inventory resources.
   * @param inventory Inventory supplying resources for the upgrade
   * @return void if successful, UpgradeRejectionType if the upgrade fails
   */
  [[nodiscard]]
  std::expected<void, Building::UpgradeRejectionType>
  AttemptUpgrade(InteractiveWorldInventory &inventory) {
    if (m_building->IsMaxLevel()) {
      return std::unexpected(Building::UpgradeRejectionType::AlreadyMaxLevel);
    }

    const auto next_upgrade = m_building->GetNextUpgradeInfo();

    assert(next_upgrade.has_value() &&
           "Non-max-level buildings must expose a next upgrade");

    int available_quantity = inventory.GetAmount(next_upgrade->item);

    auto upgrade_result =
        m_building->Upgrade(next_upgrade->item, available_quantity);

    if (!upgrade_result) {
      return std::unexpected(upgrade_result.error());
    }

    const bool removed =
        inventory.RemoveItem(next_upgrade->item, next_upgrade->quantity);

    assert(removed && "Inventory should remove validated upgrade resources");

    return {};
  }
};
} // namespace cse498
