/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class interface for all non-agent item types.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string>
#include "../WorldBase.hpp"
#include "Item.hpp"
#include "ItemConsumable.hpp"

namespace cse498 {
    class ItemConsumableSpeed : public ItemConsumable {
        private:
            int m_speed = 1;  // Multiplier for the character speed

        public:
            ItemConsumableSpeed(size_t id, const std::string & name, const std::string & image_path, int gold, 
                const WorldBase & world) : ItemConsumable(id, name, image_path, gold, world) {
                    SetImagePath("assets/items/consumables/speed.png");
                }

            bool IsSpeed() const override {return true;}

            int GetSpeed() {return m_speed;}
            void SetSpeed(int speed) {m_speed = speed;}
    };
}