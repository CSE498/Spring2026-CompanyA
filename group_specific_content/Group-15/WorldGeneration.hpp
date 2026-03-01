/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * Attributed References for the 'random_splits' function: https://eskerda.com/bsp-dungeon-generation/
 * References: https://www.roguebasin.com/index.php/Basic_BSP_Dungeon_generation 
 * References: https://en.wikipedia.org/wiki/Binary_space_partitioning
 * @brief A BSP tree
 * @note Status: PROPOSAL
 **/


 #pragma once 


 #include "RoomHolder.hpp"
 #include "BSP-Dungeon.hpp"



struct Point {
    int x, y;
};

struct LinkedRooms {
    int x1, y1;
    int x2, y2;
};

namespace cse498 {
    

    class WorldGen {
    protected:
        BSP BSP_Tree;
        RoomHolder room_holder; 
        std::vector<std::string> grid;
        std::vector<LinkedRooms> connected_rooms;

        int offset_x; //x offset of room placement
        int offset_y; //y offset of room placement
            
    public: 

        /// @brief Creates and initializes BSP Tree, RoomHolder, and grid for outputting dungeon level
        WorldGen() 
            : BSP_Tree(),
              room_holder(), 
              grid(BSP_Tree.GetHeight(), std::string(BSP_Tree.GetWidth(), '#')) 

            {}
        

        /// @brief Assigns Nodes from BSP Tree to a random pre-made room
        void AssignRooms() { 
            if (BSP_Tree.GetLeafNodes().size() == 0) {
                throw std::runtime_error("DungeonGenerator(): Leaf Nodes not populated correctly!");
            }

            int counter = 0;

            for (auto& i : BSP_Tree.GetLeafNodes()) {
                if (counter / 3 == 0) {
                    counter = 0;
                }

                room_holder.SetRoom(counter);

                if(room_holder.GetRoomWidth() <= i.width && room_holder.GetRoomHeight() <= i.height) {
                    //later
                }

                i.vector_room = *room_holder.GetRoom();

                ++counter;
            }

        }   

        /// @brief 
        void MakeRoom() { 
            
        }

        void RasterizeGrid(Node node) { 
            for(int y = 0; y < node.height) {

            }
        }

        /// @brief DFS to go through the populated BSP tree in order to connect rooms together
        /// @param node 
        /// @return 
        Point PostOrderConnect(Node node) { //Need to change Node node to idx int later 
            if (node.left_child == -1 && node.right_child == -1) {
                auto pair = CalcRoomCenter(node.vector_room); //midpoint x and y of room

                return (Point{node.x + pair.first, node.y + pair.second});
            }

            Point left = PostOrderConnect(BSP_Tree.GetBSPTree()[node.left_child]);
            Point right = PostOrderConnect(BSP_Tree.GetBSPTree()[node.right_child]);
            

            connected_rooms.push_back(LinkedRooms{left.x, left.y, right.x, right.y}); //x1,y1,x2,y2 respectively

            return left; //need to send a node upwards, allowing connectivity between nodes for linking 
        }

        /// @brief later
        /// @param room we're inputting the BSP_Tree Node's room value
        /// @return pair of coordinates 
        std::pair<int,int> CalcRoomCenter(std::vector<std::string>& room) {
            auto width = room[0].length();
            auto height = room.size();

            return std::pair<int,int>(width / 2 , height / 2);
        }


    };
 }