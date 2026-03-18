/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simple 2D Grid container
 * @note Status: PROPOSAL
 **/


#pragma once

#include <cassert>
#include <unordered_map>
#include <vector>

#include "../core/Location.hpp"
#include "../core/WorldGrid.hpp"

#include "../tools/io_utils.hpp"

constexpr size_t UNKNOWN_CELL_TYPE_ID = 0;

namespace cse498 {


  struct CellTypeModifiers { 
    bool isBreakable = false;  ///< Checks if tile is breakable (hidden walls)
    bool locked = false;       ///< Boolean checker if tile is locked (chest/door)
    bool traversable = false;  ///< Determines if Cell is traversable
    
    //... and more added depending on utility needs

    CellTypeModifiers() = default;
    CellTypeModifiers(bool breakable, bool locked, bool traversable) : isBreakable(breakable), locked(locked), traversable(traversable) {}
    

  };

  /// @brief Simple data structure to hold info about a TYPE of cell in the world.
  /// A CellType might be a wall, an open space, or a hole in the ground -- anything that makes sense.
  struct CellType {
    std::string name;  ///< Unique name for this type of cell (e.g., "wall", "tree", "moon")
    std::string desc;  ///< Full description of what this type of cell is
    char symbol;       ///< Symbol for text representations (files or interface)
    CellTypeModifiers tile_modifiers; ///< struct object to determine modifiers of the cell
    

    // DEVELOPER NOTE: More info may be needed here to describe cell types...
    // For example,is it traversable?  Does it have special properties?
  };


  /// @class WorldGridState
  /// @brief Represents a 2D grid of cells.
  /// This class provides utilities to manage, access, and modify cells within a grid.
  class WorldGridState : public WorldGrid {
  protected:
    size_t width = 0;   ///< Number of cells in each row of the grid.
    size_t height = 0;  ///< Number of rows of cells in the grid.

    // Track all of the possible types of cells that can be in this grid.
    // A "cell ID" refers to the index in this cell_types vector.
    std::vector<CellType> cell_types{}; ///< Types of cells possible in this grid.

    std::vector<size_t> cells{};  ///< Matrix of world cells, by rows, top to bottom; value is cell ID.

    // -- Helper functions --

    /// Convert an X and a Y value to the index in the vector.
    [[nodiscard]] size_t ToIndex(size_t x, size_t y) const { return x + y * width; }

    /// Convert a WorldPosition to the index in the vector.
    [[nodiscard]] size_t ToIndex(WorldPosition pos) const { return pos.CellX() + pos.CellY() * width;}


    // -- Serialize and Deserialize functions --
    // Mechanisms to efficiently save and load the exact state of the grid.
    // File format is width and height followed by all
    // values in the grid on each line thereafter.

    std::string GetTypeName() const { return "cse498::WorldGridState"; }

    /// Write the current state of this grid into the provided stream.
    void Serialize(std::ostream & os) const {
      os << width << " " << height;
      for (size_t state : cells) os << ' ' << state;
      os << std::endl;
    }

    /// Read the state of the grid out of the provided stream. 
    void Deserialize(std::istream & is) {
      is >> width >> height;
      cells.resize(width * height);
      for (size_t & state : cells) is >> state;
    }

  public:
    WorldGridState(size_t width, size_t height, size_t default_type=0)
      : width(width), height(height), cells(width*height, default_type)
    {
      // The first cell type (ID 0) is reserved for errors or non-existant
      // positions in a cell grid (e.g., for a non-rectangular world).
      AddCellType("Unknown", "This is an invalid cell type and should not be reachable.");
    }
    WorldGridState() : width(UNKNOWN_CELL_TYPE_ID), height(UNKNOWN_CELL_TYPE_ID), cells() {
      AddCellType("Unknown", "This is an invalid cell type and should not be reachable.");
    }
    WorldGridState(const WorldGridState &) = default;
    WorldGridState(WorldGridState &&) = default;
    
    WorldGridState & operator=(const WorldGridState &) = default;
    WorldGridState & operator=(WorldGridState &&) = default;

    // -- Accessors --
    [[nodiscard]] size_t GetWidth() const { return width; }
    [[nodiscard]] size_t GetHeight() const { return height; }
    [[nodiscard]] size_t GetNumCells() const { return cells.size(); }

    /// Test if specific coordinates are in range for this GridWorld.
    [[nodiscard]] bool IsValid(double x, double y) const {
      return x >= 0.0 && x < width && y >= 0.0 && y < height;
    }

    /// Test if a WorldPosition is in range for this GridWorld.
    [[nodiscard]] bool IsValid(WorldPosition pos) const {
      return IsValid(pos.X(), pos.Y());
    }

    /// @return The grid state at the provided x and y coordinates
    [[nodiscard]] size_t operator[](std::pair<size_t, size_t> coordinates) const {
      assert(IsValid(coordinates.first, coordinates.second));
      return cells[ToIndex(coordinates.first, coordinates.second)];
    }

     /// @return A reference to the grid state at the provided x and y coordinates
     [[nodiscard]] size_t & operator[](std::pair<size_t, size_t> coordinates) {
      assert(IsValid(coordinates.first, coordinates.second));
      return cells[ToIndex(coordinates.first, coordinates.second)];
     }

    [[nodiscard]] size_t operator[](WorldPosition pos) const { 
      assert(IsValid(pos));
      return cells[ToIndex(pos)]; 
    }
    [[nodiscard]] size_t & operator[](WorldPosition pos) { 
      assert(IsValid(pos));
      return cells[ToIndex(pos)]; 
    }

    // ===========================
    //   Cell type management...
    // ===========================

    [[nodiscard]] const std::vector<CellType> & GetCellTypes() const { return cell_types; }

    /// @brief  Configure a new type of cell that can be in this grid.
    /// @param name A unique name for this cell type
    /// @param desc A longer description of the cell type
    /// @param symbol An (optional) unique symbol for text IO (files, command line)
    /// @return A unique ID associated with this cell type (position in type_options vector)
    size_t AddCellType(const std::string & name, const std::string & desc="", char symbol='\0', 
                        CellTypeModifiers mod = {}) 
    {
      cell_types.push_back(CellType{name, desc, symbol, mod});
      return cell_types.size() - 1;
    }

    /// @brief Return the ID associated with the cell type name.
    /// @param name The unique name of the cell type
    /// @return The unique ID of the cell type (or 0 if it doesn't exist.)
    [[nodiscard]] size_t GetCellTypeID(const std::string & name) const { 
      for (size_t i=1; i < cell_types.size(); ++i) {
        if (cell_types[i].name == name) return i;
      }
      return 0;
    }

    [[nodiscard]] const std::string & GetCellTypeName(size_t id) const {
      if (id >= cell_types.size()) return cell_types[0].name;
      return cell_types[id].name;
    }

    [[nodiscard]] char GetCellTypeSymbol(size_t id) const {
      if (id >= cell_types.size()) return cell_types[0].symbol;
      return cell_types[id].symbol;
    }


  };



} // End of namespace cse498