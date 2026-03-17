/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class interface for all non-agent item types.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string>
#include "WorldBase.hpp"

namespace cse498 {
    class Item {
        private:
            const size_t m_id;              // An id unique to this item
            std::string m_name;             // Name of the item
            std::string m_image_path;       // Filepath to the image for this item
            int m_gold;                     // Amount of gold the item can be sold/bought for
            const WorldBase & m_world;        // World this item is in
            // Put something here for specific location? Wether or not it is in player inventory?

        public:
            Item(size_t id, const std::string & name, const std::string & image_path, int gold, 
                const WorldBase & world): m_id(id), m_name(name), m_image_path(image_path), 
                m_gold(gold), m_world(world) {}
            
            size_t GetId() {return m_id;}
            std::string GetName() {return m_name;}
            std::string GetImagePath() {return m_image_path;}
            int GetGold() {return m_gold;}

            void SetName(const std::string & name) {m_name = name;}
            void SetImagePath(const std::string & image_path) {m_image_path = image_path;}
            void SetGold(int gold) {m_gold = gold;}

            virtual bool IsWeapon() const {return false;}
            virtual bool IsConsumable() const {return false;}

            virtual void DestroyItem() {
                // TODO: Delete the item from the world
            }
    };
}