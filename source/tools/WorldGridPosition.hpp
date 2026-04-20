/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A specification of a coordinate position in a World.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>    // For assert
#include <concepts>   // For std::integral
#include <compare>    // For operator<=>
#include <cstddef>    // For size_t
#include <algorithm> //for std::max and std::min
#include "../../third-party/gsl/gsl" //For gsl::narrow_cast

#include "../source/core/WorldPosition.hpp"

namespace cse498 {
    /**
    * @class WorldGridPosition
    * @brief Represents a position within a 2D world grid with directional orientation.
    * @details ...
    */
    class WorldGridPosition : public WorldPosition {
    public:
        /**
         * @enum Orientation
         * @brief Cardinal directions for entity facing.
         */
        enum class Orientation { NORTH, SOUTH, EAST, WEST }; // enum states for direction

    private:
        Orientation m_dir = Orientation::NORTH; // Sets default value to NORTH

    public:
        using WorldPosition::WorldPosition;

        /**
         * @brief Gets the current facing direction.
         * @return Current Orientation value
         */
        [[nodiscard]] Orientation Dir() const { return m_dir; }

        /**
         * @brief Sets a new facing direction.
         * @param new_dir The new Orientation to face
         */
        void SetDir(Orientation new_dir) { m_dir = new_dir; }

        /**
         * @brief Checks if this position occupies the same cell as another.
         * @param other The other position to compare against
         * @return True if both positions are in the same cell
         */
        [[nodiscard]] bool IsColliding(const WorldPosition &other) const {
            return (CellX() == other.CellX() && CellY() == other.CellY());
        }

        /**
         * @brief Checks if two positions are in adjacent cells.
         * @param other The other position to check adjacency with
         * @return True if cells are exactly one step apart (Manhattan distance of 1)
         */
        [[nodiscard]] bool IsAdjacentCell(const WorldPosition &other) const {
            size_t dx = std::max(CellX(), other.CellX()) - std::min(CellX(), other.CellX());
            size_t dy = std::max(CellY(), other.CellY()) - std::min(CellY(), other.CellY());
            return (dx + dy == 1);
        }

        /**
         * @brief Returns a new position offset from this one.
         * @param offset_x Horizontal offset to apply
         * @param offset_y Vertical offset to apply
         * @return New WorldGridPosition at the offset location
         */
        [[nodiscard]] WorldGridPosition Offset(double offset_x, double offset_y) const {
            return WorldGridPosition{X() + offset_x, Y() + offset_y};
        }

        /**
         * @brief Calculates the Manhattan distance between two positions.
         * @param other The other position to measure distance to
         * @return Manhattan distance in cells (|dx| + |dy|)
         */
        [[nodiscard]] size_t CellDistance(const WorldPosition &other) const {
            size_t dx = std::max(CellX(), other.CellX()) - std::min(CellX(), other.CellX());
            size_t dy = std::max(CellY(), other.CellY()) - std::min(CellY(), other.CellY());
            return dx + dy;
        }
    };
} // End of namespace cse498
