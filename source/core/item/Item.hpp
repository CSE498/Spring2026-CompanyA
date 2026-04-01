/**
 * @file Item.hpp
 * @author Group2, Group15
 *
 * New Item Class that will be built out and is the decided version of the item class by group2 and group15
 * If it needs more functionality, ask one of the two groups to ensure it is consistent
 */

#pragma once

#include <string>
#include "../WorldBase.hpp"

namespace cse498
{
/**
 * Remember to set properties ESPECIALLY Unique-ness
 * Unique item == Only 1 possible in stack size and ID ids it
 */
class Item
{
private:
    const size_t m_id; // An id unique to this item
    std::string m_name; // Name of the item
    std::string m_image_path; // Filepath to the image for this item
    int m_gold; // Amount of gold the item can be sold/bought for
    [[maybe_unused]] const WorldBase &m_world; // World this item is in

public:
    Item(const size_t id,
         std::string name,
         std::string image_path,
         const int gold,
         const WorldBase &world) : m_id(id), m_name(std::move(name)), m_image_path(std::move(image_path)),
                                   m_gold(gold), m_world(world)
    {
    }

    [[nodiscard]] size_t GetId() const { return m_id; }
    [[nodiscard]] std::string GetName() const { return m_name; }
    [[nodiscard]] std::string GetImagePath() const { return m_image_path; }
    [[nodiscard]] int GetGold() const { return m_gold; }

    void SetName(const std::string &name) { m_name = name; }
    void SetImagePath(const std::string &image_path) { m_image_path = image_path; }
    void SetGold(const int gold) { m_gold = gold; }

    [[nodiscard]] virtual bool IsWeapon() const { return false; }
    [[nodiscard]] virtual bool IsConsumable() const { return false; }
    [[nodiscard]] virtual bool IsUnique() const { return false; }

    /**
     * An item is attempted to be used -- this defines what it does upon use
     * default is that it doesn't do anything
     */
    virtual void Use()
    {
    }

    virtual ~Item() = default;
};
}
