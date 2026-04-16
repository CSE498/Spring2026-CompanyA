/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class interface for all non-agent item types.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string>
#include "Item.hpp"
#include "ItemWeapon.hpp"
#include "WorldBase.hpp"

namespace cse498 {
class ItemWeaponSword : public ItemWeapon {
private:
public:
    ItemWeaponSword(size_t id, const std::string& name, const std::string& image_path, int gold,
                    const WorldBase& world) : ItemWeapon(id, name, image_path, gold, world) {
        SetImagePath("assets/items/weapons/sword.png");
    }

    bool IsSword() const override { return true; }
};
} // namespace cse498
