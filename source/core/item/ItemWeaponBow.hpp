/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class interface for all non-agent item types.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string>
#include "Item.hpp"
#include "ItemWeapon.hpp"
#include "../WorldBase.hpp"
#include "../WorldBase.hpp"

namespace cse498 {
class ItemWeaponBow : public ItemWeapon {
private:
public:
    ItemWeaponBow(size_t id, const std::string& name, const std::string& image_path, int gold, const WorldBase& world) :
        ItemWeapon(id, name, image_path, gold, world) {
        SetRange(5.0);
        SetDamage(3.0);
        SetHitBonus(0.0);
    }

    bool IsBow() const override { return true; }
};
} // namespace cse498
