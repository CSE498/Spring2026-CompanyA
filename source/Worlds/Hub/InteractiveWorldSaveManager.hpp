#pragma once

#include <fstream>
#include <string>

#include "../../../third-party/json/json.hpp"
#include "InteractiveWorld.hpp"

namespace cse498 {

using json = nlohmann::json;

class InteractiveWorldSaveManager {
public:
    // Save world to file
    bool Save(const InteractiveWorld& world, const std::string& filename) {
        json j;

        const auto& inv = world.GetInventory();

        j["inventory"]["wood"] = inv.GetAmount(ItemType::Wood);
        j["inventory"]["stone"] = inv.GetAmount(ItemType::Stone);
        j["inventory"]["metal"] = inv.GetAmount(ItemType::Metal);

        std::ofstream file(filename);

        if (!file.is_open())
            return false;

        file << j.dump(4);

        return true;
    }

    // Load world from file
    bool Load(InteractiveWorld& world, const std::string& filename) {
        std::ifstream file(filename);

        if (!file.is_open())
            return false;

        json j;
        file >> j;

        auto& inv = world.GetInventory();

        inv.Clear();

        inv.AddItem(ItemType::Wood, j["inventory"]["wood"]);

        inv.AddItem(ItemType::Stone, j["inventory"]["stone"]);

        inv.AddItem(ItemType::Metal, j["inventory"]["metal"]);

        return true;
    }
};

} // namespace cse498
