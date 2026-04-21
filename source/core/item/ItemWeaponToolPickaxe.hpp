/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class interface for all non-agent item types.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string>
#include "Item.hpp"
#include "ItemWeapon.hpp"
#include "ItemWeaponTool.hpp"
#include "WorldBase.hpp"

namespace cse498 {
class ItemWeaponToolPickaxe : public ItemWeaponTool {
public:
    ItemWeaponToolPickaxe(size_t id, const std::string& name, const std::string& image_path, int gold,
                          const WorldBase& world) : ItemWeaponTool(id, name, image_path, gold, world) {
        SetImagePath("assets/items/weapons/tools/pickaxe.png");
    }

    bool IsPickaxe() const override { return true; }
};
} // namespace cse498
