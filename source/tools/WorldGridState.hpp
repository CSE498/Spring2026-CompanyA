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
        bool isBreakable = false; ///< Checks if tile is breakable (hidden walls)
        bool locked = false; ///< Boolean checker if tile is locked (chest/door)
        bool traversable = false; ///< Determines if Cell is traversable

        CellTypeModifiers() = default;

        CellTypeModifiers(bool breakable, bool is_locked, bool is_traversable)
            : isBreakable(breakable), locked(is_locked), traversable(is_traversable) {
        }
    };

    class WorldGridState : public WorldGrid {
    protected:
        /// Parallel to WorldGrid::cell_types
        /// Index i in this vector stores the modifiers for cell type i.
        std::vector<CellTypeModifiers> mCellTypeModifiers{};

    public:
        WorldGridState(size_t width, size_t height, size_t default_type = 0)
            : WorldGrid(width, height, default_type) {
            mCellTypeModifiers.resize(GetCellTypes().size());
        }

        WorldGridState() : WorldGrid() {
            mCellTypeModifiers.resize(GetCellTypes().size());
        }

        WorldGridState(const WorldGridState &) = default;

        WorldGridState(WorldGridState &&) = default;

        WorldGridState &operator=(const WorldGridState &) = default;

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

        /// @return The grid state at the provided x and y coordinates
        [[nodiscard]] size_t operator[](std::pair<size_t, size_t> coordinates) const {
            assert(IsValid(coordinates.first, coordinates.second));
            return cells[ToIndex(coordinates.first, coordinates.second)];
        }

        /// @return A reference to the grid state at the provided x and y coordinates
        [[nodiscard]] size_t &operator[](std::pair<size_t, size_t> coordinates) {
            assert(IsValid(coordinates.first, coordinates.second));
            return cells[ToIndex(coordinates.first, coordinates.second)];
        }

        [[nodiscard]] size_t operator[](WorldPosition pos) const {
            assert(IsValid(pos));
            return cells[ToIndex(pos)];
        }

        [[nodiscard]] size_t &operator[](WorldPosition pos) {
            assert(IsValid(pos));
            return cells[ToIndex(pos)];
        }

        // -- Accessors --


        /// @brief Get modifiers for a given cell type ID.
        [[nodiscard]] const CellTypeModifiers &GetCellTypeModifiers(size_t id) const {
            static const CellTypeModifiers default_modifiers{};

            if (id >= mCellTypeModifiers.size()) {
                return default_modifiers;
            }

            return mCellTypeModifiers[id];
        }


        /// @brief Get modifiers for the cell at a given world position.
        [[nodiscard]] const CellTypeModifiers &GetCellModifiers(WorldPosition pos) const {
            return GetCellTypeModifiers((*this)[pos]);
        }

        /// @brief Check if a cell type ID is traversable.
        [[nodiscard]] bool IsTraversable(size_t id) const {
            return GetCellTypeModifiers(id).traversable;
        }

        /// @brief Check if the cell at a given position is traversable.
        [[nodiscard]] bool IsTraversable(WorldPosition pos) const {
            if (!IsValid(pos)) return false;
            return GetCellModifiers(pos).traversable;
        }

        /// @brief Check if a cell type ID is breakable.
        [[nodiscard]] bool IsBreakable(size_t id) const {
            return GetCellTypeModifiers(id).isBreakable;
        }

        /// @brief Check if the cell at a given position is breakable.
        [[nodiscard]] bool IsBreakable(WorldPosition pos) const {
            if (!IsValid(pos)) return false;
            return GetCellModifiers(pos).isBreakable;
        }

        /// @brief Check if a cell type ID is locked.
        [[nodiscard]] bool IsLocked(size_t id) const {
            return GetCellTypeModifiers(id).locked;
        }

        /// @brief Check if the cell at a given position is locked.
        [[nodiscard]] bool IsLocked(WorldPosition pos) const {
            if (!IsValid(pos)) return false;
            return GetCellModifiers(pos).locked;
        }

        /// @brief Grabs symbol at position.
        [[nodiscard]] char GetSymbol(WorldPosition pos) const {
            return GetCellTypeSymbol((*this)[pos]);
        }
    };
} // namespace cse498
