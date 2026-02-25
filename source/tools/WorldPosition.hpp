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
#include "gsl/gsl" //For gsl::narrow_cast

namespace cse498 {

  /// @class WorldPosition
  /// @brief Represents a position within a 2D world.
  /// Stored as floating point, but can be converted to coordinates
  class WorldPosition { 
  public:
    enum class Orientation { NORTH, SOUTH, EAST, WEST }; // enum states for direction

  private:
    double x = 0.0;
    double y = 0.0;
    Orientation dir = Orientation::NORTH; // Sets default value to NORTH

  public:
    WorldPosition() = default;
    WorldPosition(double x, double y) : x(x), y(y) { }
 	  template <std::integral T, std::integral U>
 	  WorldPosition(T x, U y)
   		  : WorldPosition(static_cast<double>(x), static_cast<double>(y)) {}

    WorldPosition(const WorldPosition &) = default;
    WorldPosition & operator=(const WorldPosition &) = default;

    // -- Accessors --

    /// Returns the exact floating point x coordinate
    [[nodiscard]] double X() const { return x; }
    ///Returns the exact floating point y coordinate
    [[nodiscard]] double Y() const { return y; }

    [[nodiscard]] Orientation Dir() const {return dir;} // Returns the current facing direction
    void SetDir(Orientation new_dir) {dir = new_dir;} // Sets a new facing direction

    /// Returns which grid column this position is in (3.7 -> 3)
    [[nodiscard]] size_t CellX() const {
      assert(x >= 0.0);
      return gsl::narrow_cast<size_t>(x);
    }
    /// Returns which grid row this position is in (1.2 -> 1)
    [[nodiscard]] size_t CellY() const {
      assert(y >= 0.0);
      return gsl::narrow_cast<size_t>(y);
    }

    /// Enable all comparison operators (==, !=, <, <=, >, >=)
    auto operator<=>(const WorldPosition &) const = default;


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
    [[nodiscard]] WorldPosition Offset(double offset_x, double offset_y) const {
      return WorldPosition{x+offset_x,y+offset_y};
    }

    // Gets the distance between two positions
    [[nodiscard]] size_t CellDistance(const WorldPosition& other) const
    {
       size_t dx = std::max(CellX(), other.CellX()) - std::min(CellX(), other.CellX());
       size_t dy = std::max(CellY(), other.CellY()) - std::min(CellY(), other.CellY());
       return dx + dy;
    }

    ///Returns a new position one cell up (decreasing y)
    [[nodiscard]] WorldPosition Up()    const { return {x, y-1.0}; }
    ///Returns a new position one cell down (increasing y)
    [[nodiscard]] WorldPosition Down()  const { return {x, y+1.0}; }
    ///Returns a new position one cell left (decreasing x)
    [[nodiscard]] WorldPosition Left()  const { return {x-1.0, y}; }
    ///Returns a new position one cell right (increasing x)
    [[nodiscard]] WorldPosition Right() const { return {x+1.0, y}; }
  };

} // End of namespace cse498
