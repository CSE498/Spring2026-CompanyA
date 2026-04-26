#pragma once

#include <fstream>
#include <string>

#include "../../../third-party/json/json.hpp"
#include "../../Agents/Classic/ResourceManagementAgent.hpp"
#include "InteractiveWorld.hpp"

namespace cse498 {

using json = nlohmann::json;

class InteractiveWorldSaveManager {
public:
    // Save World to file
    bool Save(const InteractiveWorld& world, const std::string& filename) {
        json j;

        const auto& inv = world.GetInventory();
        j["inventory"]["wood"] = inv.GetAmount(ItemType::Wood);
        j["inventory"]["stone"] = inv.GetAmount(ItemType::Stone);
        j["inventory"]["metal"] = inv.GetAmount(ItemType::Metal);

        j["buildings"] = json::array();

        for (const auto& buildingPtr: world.GetBuildings()) {
            if (!buildingPtr)
                continue;

            json b;
            b["name"] = buildingPtr->GetName();
            b["level"] = buildingPtr->GetCurrentLevel();

            j["buildings"].push_back(b);
        }

        for (size_t i = 0; i < world.GetNumAgents(); ++i) {
            const auto* manager = dynamic_cast<const ResourceManagementAgent*>(&world.GetAgentByIndex(i));
            if (manager == nullptr) {
                continue;
            }

            j["resource_manager"]["name"] = manager->GetName();
            j["resource_manager"]["gold"] = manager->GetGold();

            j["resource_manager"]["lanes"] = json::array();
            for (std::size_t laneIndex = 0; laneIndex < manager->GetHireableLaneCount(); ++laneIndex) {
                json laneJson;
                laneJson["label"] = manager->GetHireableLaneLabel(laneIndex);
                laneJson["unlocked"] = manager->IsLaneUnlocked(laneIndex);
                j["resource_manager"]["lanes"].push_back(laneJson);
            }

            break;
        }

        std::ofstream file(filename);
        if (!file.is_open())
            return false;

        file << j.dump(4);
        return true;
    }

    bool Load(InteractiveWorld& world, const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open())
            return false;

        json j;
        file >> j;

        auto& inv = world.GetInventory();
        inv.Clear();

        inv.AddItem(ItemType::Wood, j["inventory"].value("wood", 0));
        inv.AddItem(ItemType::Stone, j["inventory"].value("stone", 0));
        inv.AddItem(ItemType::Metal, j["inventory"].value("metal", 0));

        if (j.contains("buildings") && j["buildings"].is_array()) {
            for (const auto& savedBuilding: j["buildings"]) {
                const std::string name = savedBuilding.value("name", "");
                const int level = savedBuilding.value("level", 0);

                for (auto& buildingPtr: world.GetBuildings()) {
                    if (!buildingPtr)
                        continue;

                    if (buildingPtr->GetName() == name) {
                        if (!buildingPtr->SetCurrentLevel(level)) {
                            return false;
                        }
                        break;
                    }
                }
            }
        }

        if (j.contains("resource_manager") && j["resource_manager"].is_object()) {
            const std::string managerName = j["resource_manager"].value("name", "");
            const std::size_t gold = j["resource_manager"].value("gold", 0u);

            for (size_t i = 0; i < world.GetNumAgents(); ++i) {
                auto* manager = dynamic_cast<ResourceManagementAgent*>(&world.GetAgentByIndex(i));
                if (manager == nullptr) {
                    continue;
                }

                if (manager->GetName() == managerName) {
                    manager->SetGold(gold);
                    
                    if (j["resource_manager"].contains("lanes") &&
                        j["resource_manager"]["lanes"].is_array()) {
                        for (const auto& savedLane : j["resource_manager"]["lanes"]) {
                            const std::string label = savedLane.value("label", "");
                            const bool unlocked = savedLane.value("unlocked", false);

                            for (std::size_t laneIndex = 0; laneIndex < manager->GetHireableLaneCount(); ++laneIndex) {
                                if (manager->GetHireableLaneLabel(laneIndex) == label) {
                                    manager->SetLaneUnlocked(laneIndex, unlocked);
                                    break;
                                }
                            }
                        }
                    }
                    
                    break;
                }
            }
        }

        return true;
    }
};

} // namespace cse498
