/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class interface for all non-agent item types.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string>
#include "WorldBase.hpp"
#include "Item.hpp"
#include "ItemWeapon.hpp"
#include "ItemWeaponTool.hpp"

namespace cse498 {
    class ItemWeaponToolAxe : public ItemWeaponTool {
        public:
            ItemWeaponToolAxe(size_t id, const std::string & name, const std::string & image_path, int gold, 
                const WorldBase & world) : ItemWeaponTool(id, name, image_path, gold, world) {
                    SetImagePath("assets/items/weapons/tools/axe.png");
                }

            bool IsAxe() const override {return true;}
    };
}