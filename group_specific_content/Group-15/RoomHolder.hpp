/**
* Utilities to manage, access, and modify Rooms that are generated into the GridMap
*
 * @brief Class collection of std::vector grids for loading files
 * @note Status: PROPOSAL
 **/


 #pragma once

 #include <vector>
 #include <string>
 #include <stdexcept>
 #include <cmath>
 #include <fstream>
 #include "../tools/Random.hpp"

 const auto maze_test = (std::vector<std::string>      {"#######################",
                                                        "# #            ##     #",
                                                        "# #  #  ######    ### #",
                                                        "# #  #  #     #  #  # #",
                                                        "# #  #  #  #  #  #  # #",
                                                        "#    #     #     #    #",
                                                        "##################  # #",
                                                        "#                    ##",
                                                        "#                    ##",
                                                        "#  ####################",
                                                        "#######################"} );

const auto empty_test =(std::vector<std::string>       {"#######################",
                                                        "#                     #",
                                                        "#                     #",
                                                        "# #                   #",
                                                        "#                     #",
                                                        "#                     #",
                                                        "#                     #",
                                                        "#                    ##",
                                                        "#                    ##",
                                                        "#                     #",
                                                        "#######################"} );

const auto test_two = (std::vector<std::string>       {"#######################",
                                                        "#                     #",
                                                        "#           ####      #",
                                                        "# #                   #",
                                                        "#                     #",
                                                        "#       ####          #",
                                                        "#                     #",
                                                        "#  ##                ##",
                                                        "#            ####    ##",
                                                        "#                     #",
                                                        "#######################"} );

const auto default_room = (std::vector<std::string>    {"#######################",
                                                        "#                     #",
                                                        "# ###       ####      #",
                                                        "# #                   #",
                                                        "#     #####           #",
                                                        "#       ####          #",
                                                        "#                     #",
                                                        "#  ##                ##",
                                                        "#            ####    ##",
                                                        "#   ####              #",
                                                        "#######################"} );


namespace cse498 {

    class RoomHolder { 

    protected:
        std::vector<std::vector<std::string>> listed_rooms{default_room, test_two, empty_test}; //temp
        std::vector<std::string> *current_room = nullptr;
        std::string mFilePath = "rooms/Dungeon_";

        cse498::Random mRng;

    public:

        RoomHolder() : mRng() { 
            current_room = &listed_rooms[0];

        }

        /// @brief 
        /// @param room 
        void SetRoom(int i) {
            current_room = &listed_rooms[i];
        }

        /// @brief 
        /// @return 
        [[nodiscard]] std::vector<std::string>* GetRoom() {
            return current_room;
        }

        /// @brief 
        /// @return 
        [[nodiscard]] int GetRoomWidth() {
            assert (current_room != nullptr);
            return (*current_room)[0].length();

        }

        /// @brief 
        /// @return 
        [[nodiscard]] int GetRoomHeight() {

            assert (current_room != nullptr);
            return (*current_room).size();
        }

        /// @brief 
        /// @return 
        [[nodiscard]] std::pair<int,int> GetRoomCenter() {
            assert (current_room != nullptr);
            auto width_midpoint = std::ceil(GetRoomWidth() / 2);
            auto height_midpoint = std::ceil(GetRoomHeight() / 2);
            

            return std::make_pair(width_midpoint, height_midpoint);
        }

        [[nodiscard]] std::string GenerateFilePath() { 
            auto dungeon_select = mRng.GetInt(1,3);
            auto room_select = mRng.GetInt(1,3);
            std::string file_path = "";

            switch(dungeon_select) {
                case 1:
                    file_path += "one_pool/room_" + std::to_string(room_select) + ".txt";
                    
                    break;
                case 2:
                    file_path += "two_pool/room_" + std::to_string(room_select) + ".txt";
                    break;
                case 3:
                    file_path += "three_pool/room_" + std::to_string(room_select) + ".txt";
                    break;

                default:
                    std::cout << "Error has occurred, deafulting to Dungeon One Pool!" << std::endl;
                    file_path += "one_pool/room_" + std::to_string(room_select) + ".txt";
            }

            assert(file_path != "");
            return file_path;


        }



        /// @brief Loads a random pre-made room based off of the level dungeon we're currently in
        /// @return 
        std::vector<std::string> LoadRoom() {
            std::string selected_pool = GenerateFilePath();
            std::ifstream file(mFilePath + selected_pool); // this will open one of the 

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