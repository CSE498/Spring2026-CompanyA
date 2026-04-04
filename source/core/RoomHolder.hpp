/**
* Utilities to manage, access, and modify Rooms that are generated into the GridMap
*
 * @brief Class collection of std::vector grids for loading files
 * @note Status: PROPOSAL
 **/


 #pragma once

 #include <vector>
 #include <string>
 #include <cmath>
 #include <fstream>
 #include <iostream>
 #include <cassert>
 #include "../tools/Random.hpp"
#include "../tools/WeightedSet.hpp"

namespace cse498 {
    class BSPTree;
    class RoomHolder { 

    protected:
        std::vector<std::string> current_room; //Holds the currently selected/stored room
        std::string m_file_path; //File path used to access the directory of different .txt rooms
        std::string m_image_path = "../assets/";  //File path location for images

        cse498::Random m_rng; //Random
		cse498::WeightedSet<std::string> m_room_pool;

    public:
		RoomHolder(const cse498::WeightedSet<std::string>& room_pool,
				   const std::string& file_path) 
			: m_file_path(file_path),
			  m_rng(),
			  m_room_pool(room_pool)
		{}

        /// @brief 
        /// @return 
        [[nodiscard]] std::vector<std::string> GetCurrentRoom() {
            return current_room;
        }

        void SetCurrentRoom() { 
            auto determined_room = LoadRoom();
            current_room = determined_room;
            
        }

        /// @brief 
        /// @return 
        [[nodiscard]] int GetRoomWidth() {

            return (current_room)[0].length();

        }

        /// @brief 
        /// @return 
        [[nodiscard]] int GetRoomHeight() {

            return (current_room).size();
        }

        /// @brief 
        /// @return 
        [[nodiscard]] std::pair<int,int> GetRoomCenter() {

            auto width_midpoint = std::ceil(GetRoomWidth() / 2);
            auto height_midpoint = std::ceil(GetRoomHeight() / 2);

            return std::make_pair(width_midpoint, height_midpoint);
        }

        [[nodiscard]] std::string GenerateFilePath() { 
			auto room_select = m_rng.GetValue(0.0, m_room_pool.GetTotalWeight()).value();
			
			auto sample_result = m_room_pool.Sample(room_select);
			assert(sample_result.has_value());

			std::string file_path = sample_result.value();
            assert(file_path != "");

            return file_path;
        }


        /// @brief Picks an image file for an item/tile/agent/other creature
        /// @param tile_c is a char representing the current square of the dungeon
        std::string GetImageFile(char tile_c /*, char adj_tile*/) {
            if (!tile_c) {}

            int level = 1; // TODO: Add a counter for number of levels generated
            std::string file_path = m_image_path;

            // World objects (floor tiles, walls, doors, barriers, ect.)
            if (tile_c == '&' || tile_c == '^' || tile_c == '<' || tile_c == '>' || tile_c == '$'
                || tile_c == 'd' || tile_c == 's' || tile_c == ' ' || tile_c == 'v') {

                // Level folder
                file_path += "world/";
                if (level == 1) {file_path += "forest/";}
                else if (level == 2) {file_path += "cave/";}
                else if (level == 3) {file_path += "dungeon/";}
                else {file_path += "castle/";}

                // Floor tiles
                if (tile_c == ' ' || tile_c == 'v') {
                    file_path += "floor_tiles/tile_";

                    if (level == 1) {file_path += "grass_";}
                    else if (level == 2) {file_path += "cave_";}
                    else if (level == 3) {file_path += "stoneBrick_";}
                    else {file_path += "wood_";}

                    // Regular floor tile
                    if (tile_c == ' ') {
                        file_path += "1.png";
                        return file_path;
                    }

                    // Variant floor tiles
                    file_path += std::to_string(m_rng.GetValue(2,5).value());
                    file_path += ".png";
                    return file_path;
                }

                // Walls
                else {
                    file_path += "walls/";
                    // TODO: Internal walls/barriers, waiting for image assests to be created

                    // External walls/barriers
                    if (tile_c == '&' || tile_c == '^' || tile_c == '<' || tile_c == '>' || tile_c == '$'
                        || tile_c == 'd' || tile_c == 's') {
                            file_path += "external/";

                            // Regular door
                            if(tile_c == 'd') {file_path += "door_";}
                            // Everything else
                            else {file_path += "border_";}

                            // Position on grid
                            if (tile_c == '&') {file_path += "bottom_";}
                            else if (tile_c == '^') {file_path += "top_";}
                            else if (tile_c == '<') {file_path += "left_";}
                            else if (tile_c == '>') {file_path += "right_";}\
                            else {
                                // TODO: figure out how to determine if door is left or right
                                file_path += "left_";
                            }

                            if (level == 1) {file_path += "forest.png";}
                            else if (level == 2) {file_path += "cave.png";}
                            else if (level == 3) {file_path += "dungeon.png";}
                            else {file_path += "castle.png";}
                        }
                }
            }


            else if (tile_c == 't') {
                // TODO: Trap tile, waiting for image assests to be created
            }
            else if (tile_c == 'l') {
                // TODO: Loot tile, waiting for image assests to be created
            }
            else if (tile_c == 'm') {
                file_path += "agents/monsters/agent_monster_";

                int monster = m_rng.GetValue(1,2).value();
                if (monster == 1) {
                    file_path += "goblin.png";
                }
                else if (monster == 2) {
                    file_path += "skeleton.png";
                }
                return file_path;
            }
            else {

            }
            return file_path;
        }

        /// @brief Loads a random pre-made room based off of the level dungeon we're currently in
        /// @return 
        std::vector<std::string> LoadRoom() {
            std::string selected_pool = GenerateFilePath();
            std::ifstream file(m_file_path + selected_pool); // this will open one of the rooms
            //std::cout << m_file_path + selected_pool << std::endl;

            assert(file.is_open());

            std::vector<std::string> lines;
            std::string line;

            while (std::getline(file, line)) {
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }

                lines.push_back(line);
            }

            return lines;


        }

    };


}