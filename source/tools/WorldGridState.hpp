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

/// @brief ID used for unknown or invalid cell types
constexpr size_t UNKNOWN_CELL_TYPE_ID = 0;

namespace cse498 {
    /**
     * @struct CellTypeModifiers
     * @brief Stores modifier flags for a cell type.
     *
     * @details Contains boolean flags that determine cell behavior such as
     * whether it can be broken, is locked, or can be traversed by agents.
     */
    struct CellTypeModifiers {
        bool isBreakable = false; ///< Checks if tile is breakable (hidden walls)
        bool locked = false; ///< Boolean checker if tile is locked (chest/door)
        bool traversable = false; ///< Determines if Cell is traversable

        CellTypeModifiers() = default;

        CellTypeModifiers(bool breakable, bool is_locked, bool is_traversable)
            : isBreakable(breakable), locked(is_locked), traversable(is_traversable) {
        }
    };

    /**
     * @class WorldGridState
     * @brief Extends WorldGrid with cell type modifiers for game logic.
     *
     * @details Adds a parallel vector of CellTypeModifiers that tracks additional
     * properties for each cell type
     */
    class WorldGridState : public WorldGrid {
    protected:
        /// Parallel to WorldGrid::cell_types
        /// Index i in this vector stores the modifiers for cell type i.
        std::vector<CellTypeModifiers> mCellTypeModifiers{};

    public:
        /**
          * @brief Constructs a grid with specified dimensions.
          * @param width Number of cells in each row
          * @param height Number of rows
          * @param default_type Default cell type ID for all cells
          */
        WorldGridState(size_t width, size_t height, size_t default_type = 0)
            : WorldGrid(width, height, default_type) {
            mCellTypeModifiers.resize(GetCellTypes().size());
        }

        /**
         * @brief Default constructor creating an empty grid.
         */
        WorldGridState() : WorldGrid() {
            mCellTypeModifiers.resize(GetCellTypes().size());
        }

        /// @brief Copy constructor
        WorldGridState(const WorldGridState &) = default;

        /// @brief Move constructor
        WorldGridState(WorldGridState &&) = default;

        /// @brief Copy assignment operator
        WorldGridState &operator=(const WorldGridState &) = default;

        /// @brief Move assignment operator
        WorldGridState &operator=(WorldGridState &&) = default;


        // ===========================
        //   Cell type management...
        // ===========================

        /// @brief Add a new cell type along with its modifiers.
        /// @param name Unique cell type name
        /// @param desc Description of the cell type
        /// @param symbol Character used for printing/loading
        /// @param mod Modifier data for this cell type
        /// @return The new cell type ID
        size_t AddCellType(const std::string &name,
                           const std::string &desc = "",
                           char symbol = '\0',
                           CellTypeModifiers mod = {}) {
            size_t id = WorldGrid::AddCellType(name, desc, symbol);

            if (mCellTypeModifiers.size() <= id) {
                mCellTypeModifiers.resize(id + 1);
            }

            mCellTypeModifiers[id] = mod;
            return id;
        }

        /**
         * @brief Gets the cell type ID at the specified coordinates.
         * @param coordinates Pair of (x, y) coordinates
         * @return Cell type ID at that position
         */
        [[nodiscard]] size_t operator[](std::pair<size_t, size_t> coordinates) const {
            assert(IsValid(coordinates.first, coordinates.second));
            return cells[ToIndex(coordinates.first, coordinates.second)];
        }

        /**
         * @brief Gets a reference to the cell type ID at the specified coordinates.
         * @param coordinates Pair of (x, y) coordinates
         * @return Reference to cell type ID at that position
         */
        [[nodiscard]] size_t &operator[](std::pair<size_t, size_t> coordinates) {
            assert(IsValid(coordinates.first, coordinates.second));
            return cells[ToIndex(coordinates.first, coordinates.second)];
        }

        /**
         * @brief Gets the cell type ID at a world position.
         * @param pos World position to query
         * @return Cell type ID at that position
         */
        [[nodiscard]] size_t operator[](WorldPosition pos) const {
            assert(IsValid(pos));
            return cells[ToIndex(pos)];
        }

        /**
         * @brief Gets a reference to the cell type ID at a world position.
         * @param pos World position to query
         * @return Reference to cell type ID at that position
         */
        [[nodiscard]] size_t &operator[](WorldPosition pos) {
            assert(IsValid(pos));
            return cells[ToIndex(pos)];
        }

        // -- Accessors --


        /**
          * @brief Gets modifiers for a given cell type ID.
          * @param id Cell type ID to query
          * @return Const reference to the modifiers (default modifiers if ID out of range)
          */
        [[nodiscard]] const CellTypeModifiers &GetCellTypeModifiers(size_t id) const {
            static const CellTypeModifiers default_modifiers{};

            if (id >= mCellTypeModifiers.size()) {
                return default_modifiers;
            }

            return mCellTypeModifiers[id];
        }


        /**
         * @brief Gets modifiers for the cell at a given world position.
         * @param pos World position to query
         * @return Const reference to the modifiers at that position
         */
        [[nodiscard]] const CellTypeModifiers &GetCellModifiers(WorldPosition pos) const {
            return GetCellTypeModifiers((*this)[pos]);
        }


        /**
         * @brief Checks if a cell type ID is traversable.
         * @param id Cell type ID to check
         * @return True if the cell type is traversable
         */
        [[nodiscard]] bool IsTraversable(size_t id) const {
            return GetCellTypeModifiers(id).traversable;
        }

        /**
           * @brief Checks if a cell type ID is breakable.
           * @param id Cell type ID to check
           * @return True if the cell type is breakable
           */
        [[nodiscard]] bool IsTraversable(WorldPosition pos) const {
            if (!IsValid(pos)) return false;
            return GetCellModifiers(pos).traversable;
        }

        /**
          * @brief Checks if the cell at a given position is breakable.
          * @param pos World position to check
          * @return True if the cell is breakable, false if invalid or not breakable
          */
        [[nodiscard]] bool IsBreakable(WorldPosition pos) const {
            if (!IsValid(pos)) return false;
            return GetCellModifiers(pos).isBreakable;
        }

        /**
         * @brief Checks if a cell type ID is locked.
         * @param id Cell type ID to check
         * @return True if the cell type is locked
         */
        [[nodiscard]] bool IsLocked(size_t id) const {
            return GetCellTypeModifiers(id).locked;
        }

        /**
         * @brief Checks if the cell at a given position is locked.
         * @param pos World position to check
         * @return True if the cell is locked, false if invalid or not locked
         */
        [[nodiscard]] bool IsLocked(WorldPosition pos) const {
            if (!IsValid(pos)) return false;
            return GetCellModifiers(pos).locked;
        }

        /**
         * @brief Gets the symbol character at a position.
         * @param pos World position to query
         * @return Character symbol for the cell type at that position
         */
        [[nodiscard]] char GetSymbol(WorldPosition pos) const {
            return GetCellTypeSymbol((*this)[pos]);
        }
    };
} // namespace cse498
