#pragma once

#include <fstream>
#include <string>

#include "InteractiveWorld.hpp"
#include "../../../third-party/json/json.hpp"

namespace cse498 {

    using json = nlohmann::json;

    class InteractiveWorldSaveManager {
    public:

        //Save World to file
        bool Save(const InteractiveWorld& world, const std::string& filename) {
            json j;

            const auto& inv = world.GetInventory();
            j["inventory"]["wood"]  = inv.GetAmount(ItemType::Wood);
            j["inventory"]["stone"] = inv.GetAmount(ItemType::Stone);
            j["inventory"]["metal"] = inv.GetAmount(ItemType::Metal);

            j["buildings"] = json::array();

            for (const auto& buildingPtr : world.GetBuildings()) {
                if (!buildingPtr) continue;

                json b;
                b["name"] = buildingPtr->GetName();
                b["level"] = buildingPtr->GetCurrentLevel();

                j["buildings"].push_back(b);
            }

            std::ofstream file(filename);
            if (!file.is_open()) return false;

            file << j.dump(4);
            return true;
        }

        bool Load(InteractiveWorld& world, const std::string& filename) {
            std::ifstream file(filename);
            if (!file.is_open()) return false;

            json j;
            file >> j;

            auto& inv = world.GetInventory();
            inv.Clear();

            inv.AddItem(ItemType::Wood,  j["inventory"].value("wood", 0));
            inv.AddItem(ItemType::Stone, j["inventory"].value("stone", 0));
            inv.AddItem(ItemType::Metal, j["inventory"].value("metal", 0));

            if (j.contains("buildings") && j["buildings"].is_array()) {
                for (const auto& savedBuilding : j["buildings"]) {
                    const std::string name = savedBuilding.value("name", "");
                    const int level = savedBuilding.value("level", 0);

                    for (auto& buildingPtr : world.GetBuildings()) {
                        if (!buildingPtr) continue;

                        if (buildingPtr->GetName() == name) {
                            if (!buildingPtr->SetCurrentLevel(level)) {
                                return false;
                            }
                            break;
                        }
                    }
                }
            }

            return true;
        }
    };

} // namespace cse498


