/**
 * @file ResourceManagementAgent.hpp
 * @brief Agent that manages building upgrades and resource sales in the
 *        interactive world.
 */

#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "../../Worlds/Hub/Building.hpp"
#include "../../Worlds/Hub/InteractiveWorldInventory.hpp"
#include "../../core/AgentBase.hpp"

namespace cse498 {

class ResourceManagementAgent : public AgentBase {
public:
    using ItemCount = InteractiveWorldInventory::ItemCount;
    using GoldAmount = std::size_t;

    ResourceManagementAgent(size_t id, const std::string& name, const WorldBase& world);

    [[nodiscard]] size_t SelectAction(const WorldGrid&) override { return 0; }
    bool Interact() override;

    ResourceManagementAgent& SetInventory(std::shared_ptr<InteractiveWorldInventory> inventory);
    [[nodiscard]] std::shared_ptr<InteractiveWorldInventory> GetInventoryPtr() const { return m_inventory; }

    ResourceManagementAgent& SetManagedBuildings(const std::vector<Building*>& buildings);
    ResourceManagementAgent& AddManagedBuilding(Building& building);
    void ClearManagedBuildings();
    [[nodiscard]] const std::vector<Building*>& GetManagedBuildings() const { return m_managedBuildings; }

    [[nodiscard]] GoldAmount GetGold() const { return m_gold; }
    void SetGold(GoldAmount amount) { m_gold = amount; }
    void AddGold(GoldAmount amount) { m_gold += amount; }

    void SetSellPrice(ItemType itemType, GoldAmount price);
    [[nodiscard]] GoldAmount GetSellPrice(ItemType itemType) const;

    bool UpgradeBuilding(std::size_t buildingIndex, std::string* message = nullptr);
    bool UpgradeBuilding(Building& building, std::string* message = nullptr);
    bool SellResource(ItemType itemType, ItemCount amount, std::string* message = nullptr);

private:
    std::shared_ptr<InteractiveWorldInventory> m_inventory;
    std::vector<Building*> m_managedBuildings;
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
};

} // namespace cse498
