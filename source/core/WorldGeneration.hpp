/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * Attributed References for the 'random_splits' function: https://eskerda.com/bsp-dungeon-generation/
 * References: https://www.roguebasin.com/index.php/Basic_BSP_Dungeon_generation 
 * References: https://en.wikipedia.org/wiki/Binary_space_partitioning
 * @brief A BSP tree
 * @note Status: PROPOSAL
 **/


#pragma once 


#include "BSP-Dungeon.hpp"
#include <cmath>
#include <ranges>
#include <algorithm>

struct Point {
    int x, y; // x-y points of a room
};

struct LinkedRooms {
    int x1, y1; //first room's coordinates
    int x2, y2; //second room's coordinates
};

namespace cse498 {
    

    class WorldGen {
    protected:
        BSP mBSP; // BSP_Tree that contains information on the grid and it's dimensions
        //RoomHolder mRoomHolder;
        std::vector<std::string> mGrid; //Grid we're rasterizing information from mBSP_Tree to
        std::vector<LinkedRooms> mConnectedRooms; //The x-y coord pairs of two rooms used for connecting the room pair
        Random mRng;
        int mOffsetX; //x offset of room placement
        int mOffsetY; //y offset of room placement
            
    public: 

        /// @brief Creates and initializes BSP Tree, RoomHolder, and grid for outputting dungeon level
        WorldGen(const cse498::WeightedSet<std::string>& room_pool) 
            : mBSP(room_pool), //For now, the constructor for BSP_tree room creation is going to generate rooms immediately when initialized, will reformat as level specifications become more detailed
              //mRoomHolder(room_pool), 
              mGrid(mBSP.GetHeight(), std::string(mBSP.GetWidth(), '#')) 
            {}
        

        /// @brief Dungeon rasterized to the grid, then connected to each other after room-to-room 
        ///relationship is created through PostOrderRoomConnect()
        void CreateDungeon() { 
            auto& tree = mBSP.GetBSPTree();
            for (const auto& node : mBSP.GetLeafNodes()) {
                RasterizeGrid(node); //Populates grid with initial, unconnected rooms 
            }
            PostOrderRoomConnect(tree[0]); //populates connection between rooms for linking
            PostOrderRoomConnect(tree[0]); //Tried to implement BFS Room connection for more varied layouts, but duplicating DFS is better 

            TunnelConnectDungeon();
        }


        /// @brief Grabs the Dungeon map vector grid for loading the information in WorldGrid
        /// @return 
        [[nodiscard]] std::vector<std::string> GetDungeon() const { 
            return mGrid;
        }

        /// @brief Grabs the Generated BSP Tree in order to extract room information to extract onto the grid
        /// @return 
        [[nodiscard]] BSP GetBSP() const {
            return mBSP;
        }

        /// @brief Grabs the vector of BSPNodes that have relations have a relationship with each other 
        /// @return vector of LinkedRoom Struct
        [[nodiscard]] std::vector<LinkedRooms> GetConnectedRooms() const { 
            return mConnectedRooms;
        }

    private:
        
        /// @brief takes the leaf node's (x,y) coordinates and room information from the BSP_Tree and translates it onto mGrid
        /// @param node BSPNode filled with room information (x/y coords, room width/height, room vector string)
        void RasterizeGrid(const BSPNode& node) { 
            int room_height = node.vector_room.size();
             

            assert(room_height != 0); //Ensures room is properly assigned and not empty

            int room_width = node.vector_room[0].length();

            int base_y = node.y; //Copy of y coord
            int base_x = node.x; //Copy of x coord

            for(int y = 0; y < room_height; ++y) {
                int grid_y = base_y + y; // Grid's current location (y-axis)
                assert(grid_y >= 0 && grid_y < (int)mGrid.size());

                for (int x = 0; x < room_width; ++x) { 
                    int grid_x = base_x + x;
                    
                    assert(grid_x >= 0 && grid_x < (int)mGrid[0].size());
                    char c = node.vector_room[y][x];

                    //if (c == '#') continue; //Skips the outer outline of the room 

                    mGrid[grid_y][grid_x] = c;
                }

            }
        }

        
        /// @brief Calculate center of room placed in grid
        /// @param room we're inputting the BSP_Tree Node's room value
        /// @return pair of coordinates 
        [[nodiscard]] Point CalcRoomCenter(std::vector<std::string>& room) {
            auto width = room[0].length();
            auto height = room.size();

            return Point(width / 2 , height / 2);
        }

        /// @brief Parses through the list of Room Nodes (BSPNodes) that hava relation with each other and connects those
        void TunnelConnectDungeon() {
            for(auto &i : mConnectedRooms) { 
                ConnectTunnel(i);
            }
        }

        /**
         * @brief Calculates the distance (x-y) between two rooms) and connects a corridor tunnel between them 
         * taking into consideration their ascii symbol
         */
        void ConnectTunnel(LinkedRooms RoomCoordinates) {
            auto [x1_value, y1_value, x2_value , y2_value] = RoomCoordinates;
            auto const wall_set = {'^','>','<','&'};

            auto point_x = x2_value - x1_value;
            auto point_y = y2_value - y1_value;

            bool negative_y = false;
            bool negative_x = false;

            if (point_y < 0) {
                negative_y = true;
                point_y = std::abs(point_y);
            } 

            if (point_x < 0) {
                negative_x = true;
                point_x = std::abs(point_x);
            }

            for (int y = 0; y <= point_y; ++y) {
                int y_point;
                if (negative_y) y_point = y1_value - y;
                else y_point = y1_value + y;

                auto &y_char = mGrid[y_point][x1_value];
                auto it = std::ranges::find(wall_set, y_char);

                if (y_char == '#' || it != wall_set.end()) {
                    y_char = ' ';
                }
            }

            for (int x = 0; x <= point_x; ++x) {
                int x_point;
                if(negative_x)  x_point = x1_value -x;
                else x_point = x1_value + x;
                
                auto &x_char = mGrid[y2_value][x_point];
                auto it = std::ranges::find(wall_set, x_char);

                if (x_char == '#' || it != wall_set.end()) x_char = ' ';

            }
        }

        /// @brief DFS to go through the populated BSP tree in order to connect rooms together
        /// @param node BSPNode filled with room information (x/y coords, room width/height, room vector string)
        /// @return (x,y) pair coordinate struct of room's location in the grid
        Point PostOrderRoomConnect(BSPNode node) { //Need to change Node node to idx int later 
            if (node.left_child == -1 && node.right_child == -1) {
                auto pair = CalcRoomCenter(node.vector_room); //midpoint x and y of room

                return (Point{node.x + pair.x, node.y + pair.y});
            }

            Point left = PostOrderRoomConnect(mBSP.GetBSPTree()[node.left_child]);
            Point right = PostOrderRoomConnect(mBSP.GetBSPTree()[node.right_child]);
            

            mConnectedRooms.push_back(LinkedRooms{left.x, left.y, right.x, right.y}); //x1,y1,x2,y2 respectively

            auto gamble = mRng.GetInt(0,1); //Determines which node is returned

            //sends a node upwards, allowing connectivity between nodes for linking 
            if (gamble == 0) {
                return right; 
            }
            else {
                return left;
            }

            return left;
            
        }
    };
 };