/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief A World that consists only of walls and open cells.
 * @note Status: PROPOSAL
 **/


#pragma once

#include <cassert>

#include "../core/DungeonBase.hpp"

namespace cse498 {

	/**
   * @class DungeonOne
   * @brief A concrete dungeon world that generates rooms from a predefined pool.
   *
   * @details
   * DungeonOne extends DungeonBase and defines a fixed set of room templates
   * with associated weights. These weights determine the likelihood of each
   * room being selected during dungeon generation.
   *
   * The room pool is constructed using a WeightedSet of file paths, where each
   * file corresponds to a room layout.
   */
	class DungeonOne: public DungeonBase {
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
		inline static constexpr std::array<std::pair<std::size_t, double>, 6> ROOM_DATA{{
			{1, 10},
			{2, 10},
			{3, 10},
			{4, 10},
			{5, 10},
			{6, 1},
		}};

		/**
		 * @brief Prefix for room file paths.
		 *
		 * @details
		 * This string is prepended to each room number when constructing
		 * file paths for loading room layouts.
		 */
		inline static const std::string PREFIX = "one_pool/room_";

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
		static cse498::WeightedSet<std::string> MakeRoomPool() {
			cse498::WeightedSet<std::string> rooms;
			
			for (const auto& [num, weight] : ROOM_DATA) {
				auto result = rooms.Insert(PREFIX + std::to_string(num) + ".txt", weight);
				assert(result.has_value());
			}

			return rooms;
		}

	public:
		/**
		 * @brief Constructs a DungeonOne world.
		 *
		 * @details
		 * Initializes the base DungeonBase with a generated room pool
		 * specific to DungeonOne.
		 */
		DungeonOne() : DungeonBase(MakeRoomPool()) {}

		/**
		 * @brief Default destructor.
		 */
		~DungeonOne() = default;
	};

} // End of namespace cse498
