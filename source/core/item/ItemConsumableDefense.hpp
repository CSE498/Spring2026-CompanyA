/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class interface for all non-agent item types.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string>
#include "WorldBase.hpp"
#include "Item.hpp"
#include "ItemConsumable.hpp"

namespace cse498 {
    class ItemConsumableDefense : public ItemConsumable {
        private:
            int m_defense = 1;  // Amount of defense the item provides

        public:
            ItemConsumableDefense(size_t id, const std::string & name, const std::string & image_path, int gold, 
                const WorldBase & world) : ItemConsumable(id, name, image_path, gold, world) {
                    SetImagePath("assets/items/consumables/defense.png");
                }

            bool IsDefense() const override {return true;}

            int GetDefense() {return m_defense;}
            void SetDefense(int defense) {m_defense = defense;}
    };
}