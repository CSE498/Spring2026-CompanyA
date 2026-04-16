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
class ItemWeapon : public Item {
private:
    double m_range; // Range of the weapon (in tiles)
    double m_damage; // Amount of damage the weapon can do
    double m_hit_bonus; // Bonus the weapon has to hit

public:
    ItemWeapon(size_t id, const std::string& name, const std::string& image_path, int gold, const WorldBase& world) :
        Item(id, name, image_path, gold, world) {}

    bool IsWeapon() const override { return true; }

    virtual bool IsTool() const { return false; }
    virtual bool IsSword() const { return false; }
    virtual bool IsBow() const { return false; }

    double GetRange() { return m_range; }
    double GetDamage() { return m_damage; }
    double GetHitBonus() { return m_hit_bonus; }
    void SetRange(double range) { m_range = range; }
    void SetDamage(double damage) { m_damage = damage; }
    void SetHitBonus(double hit_bonus) { m_hit_bonus = hit_bonus; }
};
} // namespace cse498
