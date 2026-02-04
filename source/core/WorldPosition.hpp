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
#include <cstdint>
#include <functional>
#include "../../group_specific_content/Group-02/PathVector.h"

namespace cse498 {

  /// @class WorldPosition
  /// @brief Represents a position within a 2D world.
  /// Stored as floating point, but can be converted to coordinates
  class WorldPosition {
  private:
    double x = 0.0;
    double y = 0.0;
    static constexpr double eps = 1e-9; /// epsilon
    /// chops off endings of doubles and converts to ints to make more consistent/comparable doubles

  public:
    WorldPosition() = default;
    WorldPosition(double x, double y) : x(x), y(y) { }
    template <std::integral T, std::integral U>
    WorldPosition(T x, U y)
      : WorldPosition(static_cast<double>(x), static_cast<double>(y)) {}

    WorldPosition(const WorldPosition &) = default;
    WorldPosition & operator=(const WorldPosition &) = default;
    WorldPosition operator+(const PathVector& v) const { return {x + v.X(), y + v.Y()}; }
    WorldPosition operator-(const PathVector& v) const { return {x - v.X(), y - v.Y()}; }


    // -- Accessors --

    [[nodiscard]] double X() const { return x; }
    [[nodiscard]] double Y() const { return y; }
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
    /// Positions on the map are the same if this is true in essence
    auto operator==(const WorldPosition & other) const {
        return quantize(x) == quantize(other.x) && quantize(y) == quantize(other.y);
    }

    // DEVELOPER NOTE: Add a SameCell function to identify if two positions are in the same cell.

    /// Return a the WorldPosition at the requested offset.
    [[nodiscard]] WorldPosition GetOffset(double offset_x, double offset_y) const {
      return WorldPosition{x+offset_x,y+offset_y};
    }

    [[nodiscard]] WorldPosition Up()    const { return {x, y-1.0}; }
    [[nodiscard]] WorldPosition Down()  const { return {x, y+1.0}; }
    [[nodiscard]] WorldPosition Left()  const { return {x-1.0, y}; }
    [[nodiscard]] WorldPosition Right() const { return {x+1.0, y}; }
    static std::int64_t quantize(double val ) { return static_cast<std::int64_t>(std::llround(val / eps)); }


  };




} // End of namespace cse498

namespace std
{
template <>
struct hash<cse498::WorldPosition> {
    std::size_t operator()(const cse498::WorldPosition& pos) const {
        auto qx = cse498::WorldPosition::quantize(pos.X());
        auto qy = cse498::WorldPosition::quantize(pos.Y());
        //Taken from boost hash_combine https://www.boost.org/doc/libs/latest/libs/container_hash/doc/html/hash.html
        // This is not the latest version but a good enough solution
        size_t result = std::hash<std::int64_t>{}(qx);
        result ^= std::hash<std::int64_t>{}(qy) + 0x9e3779b9 + (result << 6) + (result >> 2);
        return result;
    }
};
}
