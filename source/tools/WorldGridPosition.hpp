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

  /// @class WorldPosition
  /// @brief Represents a position within a 2D world.
  /// Stored as floating point, but can be converted to coordinates
  class WorldGridPosition : public WorldPosition {
  public:
    enum class Orientation { NORTH, SOUTH, EAST, WEST }; // enum states for direction

  private:
    Orientation m_dir = Orientation::NORTH; // Sets default value to NORTH

  public:

    using WorldPosition::WorldPosition;

    [[nodiscard]] Orientation Dir() const {return m_dir;} // Returns the current facing direction

    void SetDir(Orientation new_dir) {m_dir = new_dir;} // Sets a new facing direction

    // DEVELOPER NOTE: Add a SameCell function to identify if two positions are in the same cell.
    [[nodiscard]] bool IsColliding(const WorldPosition& other) const
    {
      return (CellX() == other.CellX() && CellY() == other.CellY());
    }

    ///Check if two positions are in adjacent cells
    [[nodiscard]] bool IsAdjacentCell(const WorldPosition& other) const
    {
      size_t dx = std::max(CellX(), other.CellX()) - std::min(CellX(), other.CellX());
      size_t dy = std::max(CellY(), other.CellY()) - std::min(CellY(), other.CellY());
      return (dx + dy == 1);
    }

    /// Return a the WorldPosition at the requested offset.
    [[nodiscard]] WorldGridPosition Offset(double offset_x, double offset_y) const {
      return WorldGridPosition{X() + offset_x, Y() + offset_y};
    }

    // Gets the distance between two positions
    [[nodiscard]] size_t CellDistance(const WorldPosition& other) const
    {
       size_t dx = std::max(CellX(), other.CellX()) - std::min(CellX(), other.CellX());
       size_t dy = std::max(CellY(), other.CellY()) - std::min(CellY(), other.CellY());
       return dx + dy;
    }

  };

} // End of namespace cse498
