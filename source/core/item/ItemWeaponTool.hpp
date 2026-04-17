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
class ItemWeaponTool : public ItemWeapon {
private:
    int m_drop_rate = 1;
    int m_harvest_speed = 1;

public:
    ItemWeaponTool(size_t id, const std::string& name, const std::string& image_path, int gold,
                   const WorldBase& world) : ItemWeapon(id, name, image_path, gold, world) {
        SetHitBonus(0);
        SetDamage(1);
        SetRange(1);
    }

    bool IsTool() const override { return true; }

    virtual bool IsAxe() const { return false; }
    virtual bool IsPickaxe() const { return false; }
    virtual bool IsShovel() const { return false; }

    void SetDropRate(int drop_rate) { m_drop_rate = drop_rate; }
    void SetHarvestSpeed(int harvest_speed) { m_harvest_speed = harvest_speed; }
    int GetDropRate() { return m_drop_rate; }
    int GetHarvestSpeed() { return m_harvest_speed; }
};
} // namespace cse498
