/**
 * @file ResourceManagementAgent.hpp
 * @brief Agent that manages building upgrades and resource sales in the
 *        interactive world.
 */

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "../../Worlds/Hub/Building.hpp"
#include "../../Worlds/Hub/InteractiveWorldInventory.hpp"
#include "../../core/AgentBase.hpp"

namespace cse498 {
class FetchAgent;
/**
 * @brief Menu-driven manager for interactive-world resource economy actions.
 *
 * ResourceManagementAgent does not own the buildings or haulers it manages.
 * Those agents are owned by the WorldBase agent list and must outlive this
 * manager entry. The manager stores references internally after public setup
 * methods validate any nullable pointer inputs.
 */
class ResourceManagementAgent : public AgentBase {
public:
    using ItemCount = InteractiveWorldInventory::ItemCount;
    using GoldAmount = std::size_t;

    ResourceManagementAgent(size_t id, const std::string& name, const WorldBase& world);

    /// @brief This agent only acts through Interact(), so its turn action is no-op.
    [[nodiscard]] size_t SelectAction(const WorldGrid&) override { return 0; }
    /// @brief Opens the text management menu for upgrades, sales, and lane unlocks.
    bool Interact() override;

    /// @brief Share the world inventory used to spend, sell, and display resources.
    ResourceManagementAgent& SetInventory(std::shared_ptr<InteractiveWorldInventory> inventory);
    [[nodiscard]] std::shared_ptr<InteractiveWorldInventory> GetInventoryPtr() const { return m_inventory; }

    /// @brief Replace the managed building list. Null pointers are ignored.
    ResourceManagementAgent& SetManagedBuildings(const std::vector<Building*>& buildings, bool unlocked = true);
    /// @brief Add a single world-owned building to the managed upgrade list.
    ResourceManagementAgent& AddManagedBuilding(Building& building, bool unlocked = true);
    /// @brief Remove all managed building references.
    void ClearManagedBuildings();
    /// @brief Return whether the managed building can currently be upgraded.
    [[nodiscard]] bool IsManagedBuildingUnlocked(std::size_t buildingIndex) const;

    [[nodiscard]] GoldAmount GetGold() const { return m_gold; }
    void SetGold(GoldAmount amount) { m_gold = amount; }
    void AddGold(GoldAmount amount) { m_gold += amount; }

    /// @brief Configure how much gold one item sells for.
    void SetSellPrice(ItemType itemType, GoldAmount price);
    [[nodiscard]] GoldAmount GetSellPrice(ItemType itemType) const;

    /// @brief Spend inventory resources to upgrade a managed building by index.
    bool UpgradeBuilding(std::size_t buildingIndex, std::string* message = nullptr);
    /// @brief Spend inventory resources to upgrade a specific building.
    bool UpgradeBuilding(Building& building, std::string* message = nullptr);
    /// @brief Sell stored resources for gold at the configured item price.
    bool SellResource(ItemType itemType, ItemCount amount, std::string* message = nullptr);

    /// @brief Register an unlockable hauling lane controlled by two fetch agents.
    ResourceManagementAgent& AddHireableLane(const std::string& label,
                                         FetchAgent& firstHauler,
                                         FetchAgent& secondHauler,
                                         Building& building,
                                         GoldAmount cost,
                                         std::function<void()> onHire = {});

    /// @brief Spend gold and activate the selected hauling lane.
    bool HireLane(std::size_t laneIndex, std::string* message = nullptr);

    [[nodiscard]] std::size_t GetHireableLaneCount() const { return m_hireableLanes.size(); }
    [[nodiscard]] const std::string& GetHireableLaneLabel(std::size_t laneIndex) const;
    [[nodiscard]] GoldAmount GetHireableLaneCost(std::size_t laneIndex) const;
    [[nodiscard]] bool IsLaneUnlocked(std::size_t laneIndex) const;
    bool SetLaneUnlocked(std::size_t laneIndex, bool unlocked, std::string* message = nullptr);

private:
    std::shared_ptr<InteractiveWorldInventory> m_inventory;
    struct ManagedBuildingEntry {
        std::reference_wrapper<Building> building;
        bool unlocked = true;
    };

    std::vector<ManagedBuildingEntry> m_managedBuildings;

    GoldAmount m_gold = 0;
    GoldAmount m_woodSellPrice = 1;
    GoldAmount m_stoneSellPrice = 2;
    GoldAmount m_metalSellPrice = 3;

    [[nodiscard]] Building* TryGetManagedBuilding(std::size_t buildingIndex) const;
    [[nodiscard]] static std::string DescribeBuilding(const Building& building);
    void PrintSummary() const;
    void PrintBuildingList() const;
    void HandleUpgradeInteraction();
    void HandleSellInteraction();
    void PrintHireableLaneList() const;
    void HandleHireInteraction();

    struct HireableLaneEntry {
        std::string label;
        std::reference_wrapper<FetchAgent> firstHauler;
        std::reference_wrapper<FetchAgent> secondHauler;
        GoldAmount cost = 0;
        std::reference_wrapper<Building> building;
        std::function<void()> onHire;
        bool placementApplied = false;
    };

    std::vector<HireableLaneEntry> m_hireableLanes;

};

} // namespace cse498
