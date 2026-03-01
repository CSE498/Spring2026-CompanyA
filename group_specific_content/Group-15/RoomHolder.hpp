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

    public:

        RoomHolder() = default;
        RoomHolder() { 
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
            if (current_room == nullptr) {
                throw std::runtime_error("Get Room Width: current_room not properly initialized!");
            }
            return (*current_room)[0].length();

        }

        /// @brief 
        /// @return 
        [[nodiscard]] int GetRoomHeight() {
            if (current_room == nullptr) {
                throw std::runtime_error("Get Room height: current_room not properly initialized!");
            }
            return (*current_room).size();
        }

        /// @brief 
        /// @return 
        [[nodiscard]] std::pair<int,int> GetRoomCenter() {
            if (current_room == nullptr) {
                throw std::runtime_error("Get Room center: current_room not properly initialized!");
            }
            auto width_midpoint = std::ceil(GetRoomWidth() / 2);
            auto height_midpoint = std::ceil(GetRoomHeight() / 2);
            

            return std::make_pair(width_midpoint, height_midpoint);
        }



        /// @brief Just in case we're loading in from a txt.file TEMP TEMP TEMP
        /// @return 
        std::vector<std::string> LoadRoom(std::string& stream) {
            std::ifstream file(stream);

            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file: " + stream);
            }

            std::vector<std::string> lines;
            std::string line;

            while (std::getline(file, line)) {
                // Remove Windows CR if present
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }

                lines.push_back(line);
            }

            return lines;


        }

    };


}