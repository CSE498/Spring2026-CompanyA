/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * Attributed References for the 'random_splits' function: https://eskerda.com/bsp-dungeon-generation/
 * References: https://www.roguebasin.com/index.php/Basic_BSP_Dungeon_generation 
 * References: https://en.wikipedia.org/wiki/Binary_space_partitioning
 * @brief  Provides the utilities and tools to aid in the generation of a dungeon world
 * @note Status: PROPOSAL
 **/


#pragma once


#include "BSP-Dungeon.hpp"
#include "LevelBase.hpp"
#include <cmath>
#include <ranges>
#include <algorithm>

namespace cse498 {
    /// @brief Level 1 grid width
    constexpr int LEVEL_ONE_WIDTH = 125;
    /// @brief Level 1 grid height
    constexpr int LEVEL_ONE_HEIGHT = 80;
    /// @brief Level 1 BSP iterations
    constexpr int LEVEL_ONE_ITERATIONS = 4;

    /// @brief Level 2 grid width
    constexpr int LEVEL_TWO_WIDTH = 125;
    /// @brief Level 2 grid height
    constexpr int LEVEL_TWO_HEIGHT = 125;
    /// @brief Level 2 BSP iterations
    constexpr int LEVEL_TWO_ITERATIONS = 16;

    /// @brief Level 3 grid width
    constexpr int LEVEL_THREE_WIDTH = 150;
    /// @brief Level 3 grid height
    constexpr int LEVEL_THREE_HEIGHT = 150;
    /// @brief Level 3 BSP iterations
    constexpr int LEVEL_THREE_ITERATIONS = 20;

    /**
     * @struct Point
     * @brief Represents a 2D coordinate point.
     */
    struct Point {
        int x, y; // x-y points of a room
    };


    /**
     * @struct LinkedRooms
     * @brief Stores coordinates of two rooms that should be connected.
     */
    struct LinkedRooms {
        int x1, y1; //first room's coordinates
        int x2, y2; //second room's coordinates
    };

    /// @brief utilizes BSP-Tree.hpp and RoomHolder.hpp to generate a dungeon level, represented as a vector of strings to be loaded in DungeonBase.
    /**
     * @class WorldGeneration
     * @brief Utilizes BSP tree and RoomHolder to generate dungeon levels.
     *
     * @details This class coordinates the dungeon generation process by using
     * a BSP tree to partition space, placing rooms in leaf nodes, and then
     * connecting rooms with corridors.
     */
    class WorldGeneration {
    protected:
        BSP m_bsp; // BSP_Tree that contains information on the grid and it's dimensions
        //RoomHolder mRoomHolder;
        std::vector<std::string> m_grid; //Grid we're rasterizing information from mBSP_Tree to
        std::vector<LinkedRooms> m_connected_rooms; //The x-y coord pairs of two rooms used for connecting the room pair
        Random m_rng; //Random number generator
        // int m_offset_x; //x offset of room placement
        // int m_offset_y; //y offset of room placement

    public:
        /// @brief Creates and initializes BSP Tree, RoomHolder, and grid for outputting dungeon level
        WorldGeneration(const LevelBase &level)
            : m_bsp(level)
        
        //mRoomHolder(room_pool),
        //m_grid(m_bsp.GetHeight(), std::string(m_bsp.GetWidth(), '#'))
        {
        }


        /// @brief Dungeon rasterized to the grid, then connected to each other after room-to-roomrelationship is created through PostOrderRoomConnect()
        void CreateDungeon(int &level_value) {
            LevelManager(level_value); //Loads in the width/height parameters of level

            m_bsp.CreateBSPTree();
            auto &tree = m_bsp.GetBSPTree();
            for (const auto &node: m_bsp.GetLeafNodes()) {
                RasterizeGrid(node); //Populates grid with initial, unconnected rooms
            }
            PostOrderRoomConnect(tree[0]); //populates connection between rooms for linking
            PostOrderRoomConnect(tree[0]);
            //Tried to implement BFS Room connection for more varied layouts, but duplicating DFS is better

            TunnelConnectDungeon();
        }

        /**
         * @brief Resets and regenerates the dungeon for level progression.
         * @param level_value The new level number to generate
         */
        void ClearLevel() {
            m_grid = std::vector<std::string>(m_bsp.GetHeight(), std::string(m_bsp.GetWidth(), '#')); //reset grid
            m_bsp.ClearState(); //Clear BSP State
            m_connected_rooms.clear(); //get rid of tunneling list
            //CreateDungeon(level_value);
        }


        /// @brief Grabs the Dungeon map vector grid for loading the information in WorldGrid
        /// @return
        [[nodiscard]] std::vector<std::string> GetDungeon() const {
            return m_grid;
        }

        /// @brief Grabs the Generated BSP Tree in order to extract room information to extract onto the grid
        /// @returnBSP tree object
        [[nodiscard]] BSP GetBSP() const {
            return m_bsp;
        }

        /// @brief Grabs the vector of BSPNodes that have relations have a relationship with each other
        /// @return vector of LinkedRoom Struct
        [[nodiscard]] std::vector<LinkedRooms> GetConnectedRooms() const {
            return m_connected_rooms;
        }

    private:
        /// @brief takes the leaf node's (x,y) coordinates and room information from the BSP_Tree and translates it onto mGrid
        /// @param node BSPNode filled with room information (x/y coords, room width/height, room vector string)
        void RasterizeGrid(const BSPNode &node) {
            int room_height = node.vector_room.size();


            assert(room_height != 0); //Ensures room is properly assigned and not empty

            int room_width = node.vector_room[0].length();

            int base_y = node.y; //Copy of y coord
            int base_x = node.x; //Copy of x coord

            for (int y = 0; y < room_height; ++y) {
                int grid_y = base_y + y; // Grid's current location (y-axis)
                assert(grid_y >= 0 && grid_y < (int)m_grid.size());

                for (int x = 0; x < room_width; ++x) {
                    int grid_x = base_x + x;

                    assert(grid_x >= 0 && grid_x < (int)m_grid[0].size());
                    char c = node.vector_room[y][x];

                    //if (c == '#') continue; //Skips the outer outline of the room

                    m_grid[grid_y][grid_x] = c;
                }
            }
        }


        /// @brief Calculate center of room placed in grid
        /// @param room we're inputting the BSP_Tree Node's room value
        /// @return pair of coordinates
        [[nodiscard]] Point CalcRoomCenter(const std::vector<std::string> &room) const {
            auto width = room[0].length();
            auto height = room.size();

            return Point(width / 2, height / 2);
        }

        /// @brief Parses through the list of Room Nodes (BSPNodes) that have a relation with each other and connects those
        void TunnelConnectDungeon() {
            for (auto &i: m_connected_rooms) {
                ConnectBSPRooms(i);
            }
        }

        /**
         * @brief Connects the Rooms stored within the BSP Nodes taking into consideration their ascii symbol
         * 
         * @details Calculates the distance (x-y) between two rooms) and connects a corridor tunnel between them 
         * 
         */
        void ConnectBSPRooms(LinkedRooms RoomCoordinates) {
            auto [x1_value, y1_value, x2_value , y2_value] = RoomCoordinates;
            auto const wall_set = {'1', '2', '3', '4'};

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

                auto &y_char = m_grid[y_point][x1_value];
                auto it = std::ranges::find(wall_set, y_char);

                if (y_char == '#' || it != wall_set.end()) {
                    y_char = 'a';
                }
            }

            for (int x = 0; x <= point_x; ++x) {
                int x_point;
                if (negative_x) x_point = x1_value - x;
                else x_point = x1_value + x;

                auto &x_char = m_grid[y2_value][x_point];
                auto it = std::ranges::find(wall_set, x_char);

                if (x_char == '#' || it != wall_set.end()) x_char = 'a';
            }
        }

        /// @brief Post-order DFS algorithm that searches BSP Tree
        /// @details DFS to go through the populated BSP tree in order to connect rooms together
        /// @param node BSPNode filled with room information (x/y coords, room width/height, room vector string)
        /// @return (x,y) pair coordinate struct of room's location in the grid
        Point PostOrderRoomConnect(BSPNode node) { 
            if (node.left_child == -1 && node.right_child == -1) {
                auto pair = CalcRoomCenter(node.vector_room); //midpoint x and y of room

                return (Point{node.x + pair.x, node.y + pair.y});
            }

            Point left = PostOrderRoomConnect(m_bsp.GetBSPTree()[node.left_child]);
            Point right = PostOrderRoomConnect(m_bsp.GetBSPTree()[node.right_child]);


            m_connected_rooms.push_back(LinkedRooms{left.x, left.y, right.x, right.y}); //x1,y1,x2,y2 respectively

            auto return_determiner = m_rng.GetValue(0, 1); //Determines which node is returned

            //sends a node upwards, allowing connectivity between nodes for linking
            if (return_determiner == 0) {
                return right;
            }

            return left;
        }

        /// @brief Loads in the parameters for ELevelOne in the BSPnode Object instance
        void LevelOneState() {
            m_bsp.SetWidth(LEVEL_ONE_WIDTH);
            m_bsp.SetHeight(LEVEL_ONE_HEIGHT);
            m_bsp.SetIterations(LEVEL_ONE_ITERATIONS);
        }

        /// @brief Loads in the parameters for ELevelOne in the BSPnode Object instance
        void LevelTwoState() {
            m_bsp.SetWidth(LEVEL_TWO_WIDTH);
            m_bsp.SetHeight(LEVEL_TWO_HEIGHT);
            m_bsp.SetIterations(LEVEL_TWO_ITERATIONS);
        }

        /// @brief Loads in the parameters for ELevelOne in the BSPnode Object instance
        void LevelThreeState() {
            m_bsp.SetWidth(LEVEL_THREE_WIDTH);
            m_bsp.SetHeight(LEVEL_THREE_HEIGHT);
            m_bsp.SetIterations(LEVEL_THREE_ITERATIONS);
        }

        /// @brief Will change the width and height of the Dungeon level depending on what level the Player is im
        /// @param level_value int value that determines the level the player is currently on
        void LevelManager(int &level_value) {
            switch (level_value) {
                case 1:
                    LevelOneState();
                    break;

                case 2:
                    LevelTwoState();
                    break;
                case 3:
                    LevelThreeState();
                    break;

                default:
                    LevelOneState();
                    break;
            }
            //Creates grid of size that's dependent on LevelState
            m_grid = std::vector<std::string>(m_bsp.GetHeight(), std::string(m_bsp.GetWidth(), '#')); 

        }
    };
};
