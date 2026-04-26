/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class interface for all non-agent item types.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string>
#include "Item.hpp"
#include "../WorldBase.hpp"

namespace cse498 {
class ItemWeapon : public Item {
private:
    double m_range; // Range of the weapon (in tiles)
    double m_damage; // Amount of damage the weapon can do
    double m_hit_bonus; // Bonus the weapon has to hit

public:
    /**
     * Base weapon item constructor.
     * Initializes weapon modifiers to zero so derived weapons can opt in.
     */
    ItemWeapon(size_t id, const std::string& name, const std::string& image_path, int gold, const WorldBase& world) :
        Item(id, name, image_path, gold, world), m_range(0.0), m_damage(0.0), m_hit_bonus(0.0) {}

    bool IsWeapon() const override { return true; }

    virtual bool IsTool() const { return false; }
    virtual bool IsSword() const { return false; }
    virtual bool IsBow() const { return false; }

    /// @return Effective attack range bonus/override for this weapon.
    [[nodiscard]] double GetRange() const { return m_range; }
    /// @return Flat damage contribution this weapon adds to attacks.
    [[nodiscard]] double GetDamage() const { return m_damage; }
    /// @return Hit/chance modifier associated with this weapon.
    [[nodiscard]] double GetHitBonus() const { return m_hit_bonus; }
    void SetRange(double range) { m_range = range; }
    void SetDamage(double damage) { m_damage = damage; }
    void SetHitBonus(double hit_bonus) { m_hit_bonus = hit_bonus; }
};
} // namespace cse498
