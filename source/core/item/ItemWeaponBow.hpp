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

namespace cse498 {
    class ItemWeaponBow : public ItemWeapon {
        private:

        public:
            ItemWeaponBow(size_t id, const std::string & name, const std::string & image_path, int gold, 
                const WorldBase & world) : ItemWeapon(id, name, image_path, gold, world) {
                    SetImagePath("assets/items/weapons/bow.png");
                }

            bool IsBow() const override {return true;}
    };
}