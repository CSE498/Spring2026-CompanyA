/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief 
 * @note Status: PROPOSAL
 **/


#pragma once

#include <cassert>
#include <array>
#include "../../tools/WeightedSet.hpp"
#include "LevelBase.hpp"

namespace cse498 {
    /**
   * @class CaveLevel
   * @brief
   *
   * @details
   *
   */
    class CaveLevel : public LevelBase {
    private:
        /**
        * @brief Static room ID and weight pairs.
        *
        * @details
        * Each pair consists of a room number and its associated weight.
        * The weight determines the probability of selecting that room
        * during generation.
        * Higher weights (10) make a room more likely to be selected.
        * Lower weights (1) make it less likely.
        */
        inline static constexpr std::array<std::pair<std::size_t, double>, 6> ROOM_DATA{
            {
                {1, 10},
                {2, 10},
                {3, 10},
                {4, 10},
                {5, 10},
                {6, 1},
            }
        };

        /**
         * @brief Room file paths for cave level.
         *
         * @details
         * This string is used to load cave rooms
         */
        inline static const std::string m_room_dir =
                static_cast<std::string>(DUNGEON_ROOMS_DIR) + "/Dungeon_two_pool/room_";

        /**
         * @brief Constructs the weighted room pool.
         *
         * @details
         * Iterates through ROOM_DATA and inserts each room into a
         * WeightedSet using its corresponding file path and weight.
         *
         * @return A WeightedSet containing room file paths and weights.
         *
         * @note Each insertion is expected to succeed. Assertions are used
         * to enforce this during development.
         */
        static cse498::WeightedSet<int> MakeRoomPool() {
            cse498::WeightedSet<int> rooms;

            for (const auto &[num, weight]: ROOM_DATA) {
                auto result = rooms.Insert(num, weight);
                assert(result.has_value());
            }

            return rooms;
        }

        cse498::WeightedSet<int> m_room_pool;

    public:
        /**
         * @brief Constructs a CaveLevel.
         *
         * @details
         * Initializes the base CaveLevel with a generated room pool
         */
        CaveLevel()
            : m_room_pool(MakeRoomPool()) {
        }

        /**
         * @brief Default destructor.
         */
        ~CaveLevel() = default;

        /**
         *
         */
        [[nodiscard]] const cse498::WeightedSet<int> &GetRoomPool() const override {
            return m_room_pool;
        }

        [[nodiscard]] const std::string &GetRoomDir() const override {
            return m_room_dir;
        }
    };
} // End of namespace cse498
