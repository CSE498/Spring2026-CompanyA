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
#include <cstdlib>   // For std::abs

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

    [[nodiscard]] double X() const { return x; }
    [[nodiscard]] double Y() const { return y; }

    [[nodiscard]] Orientation GetDir() const {return dir;} // Returns the current facing direction
    void SetDir(Orientation new_dir) {dir = new_dir;} // Sets a new facing direction

    [[nodiscard]] size_t CellX() const {
      assert(x >= 0.0);
      return static_cast<size_t>(x);
    }
    [[nodiscard]] size_t CellY() const {
      assert(y >= 0.0);
      return static_cast<size_t>(y);
    }

    /// Enable all comparison operators (==, !=, <, <=, >, >=)
    auto operator<=>(const WorldPosition &) const = default;


    // DEVELOPER NOTE: Add a SameCell function to identify if two positions are in the same cell.
    [[nodiscard]] bool isColliding(const WorldPosition& other) const
    {
      return (CellX() == other.CellX() && CellY() == other.CellY());
    }

    ///Check if two positions are in adjacent cells
    [[nodiscard]] bool IsAdjacentCell(const WorldPosition& other) const
    {
      int dx = std::abs(static_cast<int>(CellX()) - static_cast<int>(other.CellX()));
      int dy = std::abs(static_cast<int>(CellY()) - static_cast<int>(other.CellY()));
      return (dx + dy == 1); // Adjacent if they differ by exactly one cell in either direction
    }

    /// Return a the WorldPosition at the requested offset.
    [[nodiscard]] WorldPosition GetOffset(double offset_x, double offset_y) const {
      return WorldPosition{x+offset_x,y+offset_y};
    }

    // Gets the distance between two positions
    [[nodiscard]] int CellDistance(const WorldPosition& other) const
    {
       int dx = std::abs(static_cast<int>(CellX()) - static_cast<int>(other.CellX()));
       int dy = std::abs(static_cast<int>(CellY()) - static_cast<int>(other.CellY()));
       return (dx + dy);
    }


    [[nodiscard]] WorldPosition Up()    const { return {x, y-1.0}; }
    [[nodiscard]] WorldPosition Down()  const { return {x, y+1.0}; }
    [[nodiscard]] WorldPosition Left()  const { return {x-1.0, y}; }
    [[nodiscard]] WorldPosition Right() const { return {x+1.0, y}; }
  };

} // End of namespace cse498
