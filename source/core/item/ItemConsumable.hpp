/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class interface for all non-agent item types.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string>
#include "Item.hpp"
#include "WorldBase.hpp"

namespace cse498 {
class ItemConsumable : public Item {
private:
    int m_charges = 1; // Amount of charges the consumable has left
    int m_duration = 0; // Amount of turns the effect will last

public:
    ItemConsumable(size_t id, const std::string& name, const std::string& image_path, int gold,
                   const WorldBase& world) : Item(id, name, image_path, gold, world) {}

    bool IsConsumable() const override { return true; }

    int GetCharges() { return m_charges; }
    int GetDuration() { return m_duration; }
    void DecrementCharges(int decrement = 1) {
        m_charges -= decrement;
        if (m_charges <= 0) {
            DestroyItem();
        }
    }
    void SetCharges(int charges) { m_charges = charges; }
    void SetDuration(int duration) { m_duration = duration; }

    virtual bool IsHealing() const { return false; }
    virtual bool IsSpeed() const { return false; }
    virtual bool IsDefense() const { return false; }
};
} // namespace cse498
