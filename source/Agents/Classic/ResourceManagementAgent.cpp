/**
 * @file ResourceManagementAgent.cpp
 * @brief Interactive world agent for upgrading buildings and selling resources.
 */

#include "ResourceManagementAgent.hpp"

#include <iostream>
#include <limits>
#include <optional>

namespace cse498 {

namespace {

std::optional<std::size_t> ParseMenuIndex(const std::string& input) {
    try {
        const std::size_t parsed = std::stoull(input);
        if (parsed == 0) {
            return std::nullopt;
        }
        return parsed;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<ItemType> ParseResourceChoice(char input) {
    switch (input) {
        case 'w':
        case 'W':
            return ItemType::Wood;
        case 's':
        case 'S':
            return ItemType::Stone;
        case 'm':
        case 'M':
            return ItemType::Metal;
    }

    return std::nullopt;
}

} // namespace

ResourceManagementAgent::ResourceManagementAgent(size_t id, const std::string& name, const WorldBase& world) :
    AgentBase(id, name, world) {}

ResourceManagementAgent& ResourceManagementAgent::SetInventory(std::shared_ptr<InteractiveWorldInventory> inventory) {
    m_inventory = std::move(inventory);
    return *this;
}

ResourceManagementAgent& ResourceManagementAgent::SetManagedBuildings(const std::vector<Building*>& buildings) {
    m_managedBuildings.clear();

    for (Building* building: buildings) {
        if (building != nullptr) {
            m_managedBuildings.push_back(building);
        }
    }

    return *this;
}

ResourceManagementAgent& ResourceManagementAgent::AddManagedBuilding(Building& building) {
    m_managedBuildings.push_back(&building);
    return *this;
}

void ResourceManagementAgent::ClearManagedBuildings() { m_managedBuildings.clear(); }

void ResourceManagementAgent::SetSellPrice(ItemType itemType, GoldAmount price) {
    switch (itemType) {
        case ItemType::Wood:
            m_woodSellPrice = price;
            break;
        case ItemType::Stone:
            m_stoneSellPrice = price;
            break;
        case ItemType::Metal:
            m_metalSellPrice = price;
            break;
    }
}

ResourceManagementAgent::GoldAmount ResourceManagementAgent::GetSellPrice(ItemType itemType) const {
    switch (itemType) {
        case ItemType::Wood:
            return m_woodSellPrice;
        case ItemType::Stone:
            return m_stoneSellPrice;
        case ItemType::Metal:
            return m_metalSellPrice;
    }

    return 0;
}

Building* ResourceManagementAgent::TryGetManagedBuilding(const std::size_t buildingIndex) const {
    if (buildingIndex >= m_managedBuildings.size()) {
        return nullptr;
    }

    return m_managedBuildings[buildingIndex];
}

std::string ResourceManagementAgent::DescribeBuilding(const Building& building) {
    std::string description = building.GetName() + " | level " + std::to_string(building.GetCurrentLevel()) + "/" +
                              std::to_string(building.GetMaxLevel()) + " | next: ";

    const auto nextUpgrade = building.GetNextUpgradeInfo();
    if (!nextUpgrade.has_value()) {
        description += "max level";
        return description;
    }

    description += std::to_string(nextUpgrade->quantity) + " " + std::string(ItemTypeToString(nextUpgrade->item));
    return description;
}

void ResourceManagementAgent::PrintSummary() const {
    std::cout << "\n=== Resource Management ===\n";
    std::cout << "Stored resources: " << m_inventory->GetAmount(ItemType::Wood) << ' '
              << ItemTypeToString(ItemType::Wood) << " | " << m_inventory->GetAmount(ItemType::Stone) << ' '
              << ItemTypeToString(ItemType::Stone) << " | " << m_inventory->GetAmount(ItemType::Metal) << ' '
              << ItemTypeToString(ItemType::Metal) << '\n';
    std::cout << "Gold: " << m_gold << '\n';
}

void ResourceManagementAgent::PrintBuildingList() const {
    std::cout << "\nManaged buildings:\n";

    for (std::size_t i = 0; i < m_managedBuildings.size(); ++i) {
        const Building* building = m_managedBuildings[i];
        if (building == nullptr) {
            continue;
        }

        std::cout << " " << (i + 1) << ". " << DescribeBuilding(*building) << '\n';
    }
}

bool ResourceManagementAgent::UpgradeBuilding(std::size_t buildingIndex, std::string* message) {
    Building* building = TryGetManagedBuilding(buildingIndex);
    if (building == nullptr) {
        if (message != nullptr) {
            *message = "Unknown building selection.";
        }
        return false;
    }

    return UpgradeBuilding(*building, message);
}

bool ResourceManagementAgent::UpgradeBuilding(Building& building, std::string* message) {
    if (m_inventory == nullptr) {
        if (message != nullptr) {
            *message = "Resource manager is missing its inventory.";
        }
        return false;
    }

    const auto nextUpgrade = building.GetNextUpgradeInfo();
    if (!nextUpgrade.has_value()) {
        if (message != nullptr) {
            *message = building.GetName() + " is already at max level.";
        }
        return false;
    }

    if (!m_inventory->HasEnough(nextUpgrade->item, nextUpgrade->quantity)) {
        if (message != nullptr) {
            *message = "Not enough " + std::string(ItemTypeToString(nextUpgrade->item)) + " to upgrade " +
                       building.GetName() + ".";
        }
        return false;
    }

    if (!m_inventory->RemoveItem(nextUpgrade->item, nextUpgrade->quantity)) {
        if (message != nullptr) {
            *message = "Failed to spend resources for the upgrade.";
        }
        return false;
    }

    auto result = building.Upgrade(nextUpgrade->item, static_cast<int>(nextUpgrade->quantity));
    if (!result.has_value()) {
        m_inventory->AddItem(nextUpgrade->item, nextUpgrade->quantity);

        if (message != nullptr) {
            *message = "Upgrade failed: " + std::string(Building::UpgradeRejectionTypeToString(result.error())) + ".";
        }
        return false;
    }

    if (message != nullptr) {
        *message = building.GetName() + " upgraded to level " + std::to_string(building.GetCurrentLevel()) + '.';
    }
    return true;
}

bool ResourceManagementAgent::SellResource(ItemType itemType, ItemCount amount, std::string* message) {
    if (m_inventory == nullptr) {
        if (message != nullptr) {
            *message = "Resource manager is missing its inventory.";
        }
        return false;
    }

    if (amount == 0) {
        if (message != nullptr) {
            *message = "Sell amount must be at least 1.";
        }
        return false;
    }

    if (!m_inventory->HasEnough(itemType, amount)) {
        if (message != nullptr) {
            *message = "Not enough " + std::string(ItemTypeToString(itemType)) + " to sell.";
        }
        return false;
    }

    if (!m_inventory->RemoveItem(itemType, amount)) {
        if (message != nullptr) {
            *message = "Failed to remove resources from storage.";
        }
        return false;
    }

    const GoldAmount payout = GetSellPrice(itemType) * amount;
    m_gold += payout;

    if (message != nullptr) {
        *message = "Sold " + std::to_string(amount) + " " + std::string(ItemTypeToString(itemType)) + " for " +
                   std::to_string(payout) + " gold.";
    }
    return true;
}

void ResourceManagementAgent::HandleUpgradeInteraction() {
    PrintBuildingList();
    std::cout << "\nSelect building number to upgrade or q to cancel.\n> ";

    std::string selection;
    if (!(std::cin >> selection)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Upgrade cancelled.\n";
        return;
    }

    if (selection == "q" || selection == "Q") {
        std::cout << "Upgrade cancelled.\n";
        return;
    }

    const auto buildingNumber = ParseMenuIndex(selection);
    if (!buildingNumber.has_value()) {
        std::cout << "Invalid building selection.\n";
        return;
    }

    std::string message;
    UpgradeBuilding(*buildingNumber - 1, &message);
    std::cout << message << '\n';
}

void ResourceManagementAgent::HandleSellInteraction() {
    std::cout << "\nSell resource: [w] wood (" << GetSellPrice(ItemType::Wood) << " gold each)"
              << "  [s] stone (" << GetSellPrice(ItemType::Stone) << " gold each)"
              << "  [m] metal (" << GetSellPrice(ItemType::Metal) << " gold each)"
              << "  [q] cancel\n> ";

    char choice = '\0';
    if (!(std::cin >> choice)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Sale cancelled.\n";
        return;
    }

    if (choice == 'q' || choice == 'Q') {
        std::cout << "Sale cancelled.\n";
        return;
    }

    const auto itemType = ParseResourceChoice(choice);
    if (!itemType.has_value()) {
        std::cout << "Unknown resource selection.\n";
        return;
    }

    std::cout << "Amount to sell:\n> ";
    std::string amountInput;
    if (!(std::cin >> amountInput)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Sale cancelled.\n";
        return;
    }

    const auto amount = ParseMenuIndex(amountInput);
    if (!amount.has_value()) {
        std::cout << "Invalid sell amount.\n";
        return;
    }

    std::string message;
    SellResource(*itemType, *amount, &message);
    std::cout << message << '\n';
}

bool ResourceManagementAgent::Interact() {
    if (m_inventory == nullptr) {
        std::cout << GetName() << " is not configured with a world inventory.\n";
        return true;
    }

    while (true) {
        PrintSummary();
        PrintBuildingList();

        std::cout << "\nChoose action: [u] upgrade  [s] sell resources  [q] cancel\n> ";

        char choice = '\0';
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Closing resource management menu.\n";
            return true;
        }

        switch (choice) {
            case 'u':
            case 'U':
                HandleUpgradeInteraction();
                break;
            case 's':
            case 'S':
                HandleSellInteraction();
                break;
            case 'q':
            case 'Q':
                std::cout << "Closing resource management menu.\n";
                return true;
            default:
                std::cout << "Invalid management option.\n";
                break;
        }
    }
}

} // namespace cse498
