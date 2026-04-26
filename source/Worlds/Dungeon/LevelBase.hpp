/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief 
 * @note Status: PROPOSAL
 **/


#pragma once

#include <cassert>
#include <array>
#include "../../tools/WeightedSet.hpp"

namespace cse498 {
    /**
     * @class LevelBase
     * @brief Abstract base class defining the interface for dungeon level configurations.
     *
     * @details Derived classes must implement methods to provide the room pool
     */
    class LevelBase {
    public:
        virtual ~LevelBase() = default;

        [[nodiscard]] virtual const cse498::WeightedSet<int> &GetRoomPool() const = 0;

        [[nodiscard]] virtual const std::string &GetRoomDir() const = 0;

        //[[nodiscard]] virtual int GetWidth() const = 0;
        //[[nodiscard]] virtual int GetHeight() const = 0;
    };
} // End of namespace cse498
