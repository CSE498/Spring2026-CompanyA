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
class ItemConsumableHealing : public ItemConsumable {
private:
    int m_healing = 1; // Amount of healing that the item does

public:
    ItemConsumableHealing(size_t id, const std::string& name, const std::string& image_path, int gold,
                          const WorldBase& world) : ItemConsumable(id, name, image_path, gold, world) {}

    bool IsHealing() const override { return true; }

    int GetHealing() { return m_healing; }
    void SetHealing(int healing) { m_healing = healing; }
};
} // namespace cse498
