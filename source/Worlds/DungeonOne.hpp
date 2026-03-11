/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A World that consists only of walls and open cells.
 * @note Status: PROPOSAL
 **/


#pragma once

#include <cassert>

#include "../core/DungeonBase.hpp"

namespace cse498 {

  class DungeonOne: public DungeonBase {
    private:
    static cse498::WeightedSet<std::string> MakeRoomPool() {
        cse498::WeightedSet<std::string> rooms;
		std::string file_name = "one_pool/room_";
        rooms.Insert(file_name + "1.txt", 10);
        rooms.Insert(file_name + "2.txt", 10);
		rooms.Insert(file_name + "3.txt", 10);
		rooms.Insert(file_name + "4.txt", 10);
		rooms.Insert(file_name + "5.txt", 10);
		rooms.Insert(file_name + "6.txt", 1);
        return rooms;
    }

  public:
	DungeonOne() : DungeonBase(MakeRoomPool()) {}
    ~DungeonOne() = default;
  };

} // End of namespace cse498
