/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Represents an upgradable building in the interactive game world.
 **/

#pragma once

#include "ItemType.hpp"
#include "ResourceBank.hpp"

#include <cassert>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cse498 {
/// @class Building
/// @brief Upgradable resource building. Starts at level 0 and can also store
///        hauled resources for its production lane.
class Building : public ResourceBank {
public:
    /// Default production multiplier added per building level.
    static constexpr float DEFAULT_RATE_MODIFIER = 0.25f;

    /**
     * @struct BuildingUpgrade
     * @brief Resource cost required to advance one building level.
     */
    struct BuildingUpgrade {
        ItemType item; ///< Resource type required for the upgrade.
        int quantity; ///< Quantity required for the upgrade.
    };

    /// Reason for rejecting an attempted upgrade.
    enum class UpgradeRejectionType { AlreadyMaxLevel, IncorrectItemType, NotEnoughItems };

    /**
     * @brief Convert an upgrade rejection reason to display text.
     * @param rejection Rejection value to describe.
     * @return Human-readable reason.
     */
    static constexpr std::string_view UpgradeRejectionTypeToString(UpgradeRejectionType rejection) {
        switch (rejection) {
            case UpgradeRejectionType::AlreadyMaxLevel:
                return "building is already at max level";
            case UpgradeRejectionType::IncorrectItemType:
                return "incorrect item type for this upgrade";
            case UpgradeRejectionType::NotEnoughItems:
                return "not enough items for this upgrade";
        }

        return "unknown upgrade failure";
    }

private:
    int m_level{}; ///< Current building level.
    float m_rateModifier = DEFAULT_RATE_MODIFIER; ///< Per-level production multiplier, e.g. 0.25 for +25%.
    std::vector<BuildingUpgrade> m_upgrades{}; ///< Upgrade cost per level.

    /**
     * Get the next upgrade BuildingUpgrade struct without checking if one exists.
     * Internal helper for callers that have already verified the building is not
     * at max level.
     * @return reference to the next upgrade BuildingUpgrade struct
     */
    const BuildingUpgrade& GetNextUpgradeUnchecked() const {
        assert(m_level < GetMaxLevel() && "Building already max level!");
        return m_upgrades[m_level];
    }

    /**
     * Validate whether an upgrade can be applied to the building.
     * @param itemType The type of item offered for the upgrade.
     * @param quantity The quantity available to spend.
     * @return void if the upgrade is successful, UpgradeRejectionType describing
     * why it failed
     */
    [[nodiscard]] std::expected<void, UpgradeRejectionType> ValidateUpgrade(const ItemType& itemType,
                                                                            int quantity) const {
        if (m_level >= GetMaxLevel())
            return std::unexpected(UpgradeRejectionType::AlreadyMaxLevel);

        const auto& cost = GetNextUpgradeUnchecked();

        if (itemType != cost.item)
            return std::unexpected(UpgradeRejectionType::IncorrectItemType);

        if (quantity < cost.quantity)
            return std::unexpected(UpgradeRejectionType::NotEnoughItems);

        return {};
    }

public:
    Building() = delete;
    /**
     * @brief Construct an upgradable resource building.
     * @param id Unique id for the building.
     * @param name Name of the building.
     * @param world World this building belongs to.
     */
    Building(size_t id, const std::string& name, const WorldBase& world) : ResourceBank(id, name, world) {}
    /**
     * Get Max level for this building
     * @return max level as an int
     */
    [[nodiscard]] int GetMaxLevel() const { return m_upgrades.size(); }
    /**
     * Get current building level
     * @return current level as int
     */
    [[nodiscard]] int GetCurrentLevel() const { return m_level; }
    /**
     * Set the current building level
     * @param level level to set building to
     * @return bool if setting level was successful
     */
    [[nodiscard]] bool SetCurrentLevel(int level) {
        if (level < 0 || level > GetMaxLevel())
            return false;
        m_level = level;
        return true;
    }
    size_t SelectAction(const WorldGrid&) override { return 0; }
    /**
     * Get Next Upgrade level
     * For UI. Returns the level the next upgrade would reach. If the building is
     * already max level, returns the current level.
     * @return next upgrade level as an int
     */
    [[nodiscard]] int GetNextUpgradeLevel() const {
        if (m_level >= GetMaxLevel())
            return m_level;
        return m_level + 1;
    }
    /**
     * Check if the current level is the max level
     * @return bool if the building is max level or not
     */
    [[nodiscard]] bool IsMaxLevel() const { return m_level >= GetMaxLevel(); }
    /**
     * @brief Set the per-level production modifier used by ResourceProducer.
     * @param rate Modifier to set, where 0.25 means +25% output per level.
     */
    void SetRateModifier(float rate) { m_rateModifier = rate; }
    /**
     * @brief Get the per-level production modifier.
     * @return Rate modifier.
     */
    [[nodiscard]] float GetRateModifier() const { return m_rateModifier; }
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
     * @brief Apply the next upgrade level if the provided resource payment is valid.
     *
     * The caller is responsible for removing spent resources from the owning
     * inventory. This method only validates the supplied payment and increments
     * the building level.
     *
     * @param itemType Type of item to use.
     * @param quantity Number of items available for payment.
     * @return void if successful, otherwise an UpgradeRejectionType.
     */
    [[nodiscard]] std::expected<void, UpgradeRejectionType> Upgrade(const ItemType& itemType, int quantity) {
        auto result = ValidateUpgrade(itemType, quantity);

        if (!result)
            return std::unexpected(result.error());

        m_level++;
        return {};
    }

    /**
     * Get all BuildingUpgrade structs for the building. For UI.
     * @return vector of all of the Upgrades
     */
    [[nodiscard]] const std::vector<BuildingUpgrade>& GetAllUpgrades() const { return m_upgrades; }
    /**
     * Get the BuildingUpgrade struct for the next upgrade. Safe accessor for UI
     * and other callers that may query a max-level building.
     * @return the next BuildingUpgrade struct if it exists
     */
    [[nodiscard]] std::optional<BuildingUpgrade> GetNextUpgradeInfo() const {
        if (m_level >= GetMaxLevel())
            return std::nullopt;
        return GetNextUpgradeUnchecked();
    }
};
} // namespace cse498
