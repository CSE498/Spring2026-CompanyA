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
	 * @class ForestLevel
	 * @brief Forest-themed dungeon level providing room pool and directory.
	 *
	 * @details Implements LevelBase to provide forest-specific room configurations.
	 * Rooms are weighted to control spawn probability, with standard rooms
	 * having higher weights than special rooms.
	 */
	class ForestLevel : public LevelBase{
		private:

        /**
         * @brief Prefix for room file paths.
         *
         * @details
         * This string is prepended to each room number when constructing
         * file paths for loading room layouts.
         */
        inline static const std::string PREFIX = "one_pool/room_";

		/**
		 * @brief Room file paths for forest level.
		 *
		 * @details
		 * This string is used to load forest rooms
		 */
		inline static const std::string m_room_dir = static_cast<std::string>(DUNGEON_ROOMS_DIR) + "/Dungeon_one_pool/room_";

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
			
			for (const auto& [num, weight] : ROOM_DATA) {
				auto result = rooms.Insert(num, weight);
				assert(result.has_value());
			}

            return rooms;
        }

		cse498::WeightedSet<int> m_room_pool;

	public:
		/**
		 * @brief Constructs a ForestLevel.
		 *
		 * @details
		 * Initializes the base ForestLevel with a generated room pool
		 */
		ForestLevel()
		: m_room_pool(MakeRoomPool()) {}

		/**
		 * @brief Default destructor.
		 */
		~ForestLevel() = default;

		/**
		 * 
		 */
		[[nodiscard]] const cse498::WeightedSet<int>& GetRoomPool() const override {
    		return m_room_pool;
		}

		[[nodiscard]] const std::string& GetRoomDir() const override {
			return m_room_dir;
		}
	};

} // End of namespace cse498
