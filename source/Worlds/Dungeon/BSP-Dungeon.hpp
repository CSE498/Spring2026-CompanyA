/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * Attributed References for the 'random_splits' function: https://eskerda.com/bsp-dungeon-generation/
 * References: https://www.roguebasin.com/index.php/Basic_BSP_Dungeon_generation 
 * References: https://en.wikipedia.org/wiki/Binary_space_partitioning
 * @brief A BSP tree
 * @note Status: PROPOSAL
 **/


#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <random>
#include <string>
#include <optional>
#include <concepts>
#include <cassert>
#include <print>
#include "RoomHolder.hpp"
#include "../../tools/Random.hpp"



namespace cse498 {
    /// @brief Holds basic information of the grids
    struct BSPNode {
        
        int left_child = -1; //Left node of the tree
        int right_child = -1; //right node of the tree
        int x, y, width, height; // (x,y) - (origin point of the grid (top-left corner))
                                 // (width, height) - dimension of the grid cut (lxw)
        std::string name; //Name of the node for debugging purposes
        std::string file_name; //
        std::vector<std::string> vector_room{}; //Stores a certain dungeon room
        

        BSPNode() = default;
        BSPNode(int l, int r, int x, int y, int width, int height, std::string name) 
           : left_child(l), right_child(r), x(x), y(y), width(width), height(height), name(name) {}

    };
    

    static constexpr int DEFAULT_WIDTH = 150;
    static constexpr int DEFAULT_HEIGHT = 100;
    static constexpr int DEFAULT_WIDTH_THRESHOLD = 30;
    static constexpr int DEFAULT_HEIGHT_THRESHOLD = 20;
    static constexpr int DEFAULT_ITERATIONS = 20;

    ///Class which handles the creation, management, and modification of a Binary Space Partition (BSP) Tree in its dungeon creation
    class BSP {
    protected:
        RoomHolder m_room_holder; //Holds the rooms that populates the BSPNodes
        std::vector<BSPNode> m_BSP_tree; //The entire tree
        std::vector<BSPNode> m_leaf_nodes; //the split region tile rooms

        int m_width = DEFAULT_WIDTH; //width of the grid (MAY NEED TO MAKE CONST)
        int m_height = DEFAULT_HEIGHT; //hegiht of the grid (MAY NEED TO MAKE CONST)
        const int m_threshold_width_value = DEFAULT_WIDTH_THRESHOLD; //minimum width value before BSP stops splitting
        const int m_threshold_height_value = DEFAULT_HEIGHT_THRESHOLD;
        int m_iterations = DEFAULT_ITERATIONS; //number of splits into the grid
        Random m_rng; //Random number generator


    public: 
        /// @brief Constructor call creates the BSP Tree from the get-go, meaning that BSP_Tree and its leaf nodes are already populated 
        /// @param room_pool A pool of different rooms, each with a unique weight value, used to populate the dungeon room
        BSP(const cse498::WeightedSet<std::string>& room_pool,
			const std::string& file_path = std::string(DUNGEON_ROOMS_DIR) + "/Dungeon_") 
			: m_room_holder(room_pool, file_path)
		{ 
            insert_split(m_iterations); //Creates BSP Tree
            PostOrderDFS(); //Grabs all the generated room slots from the tree
        }

        /// @brief Constructor call creates BSP Tree with a set seed
        /// @attention This constructor is purely meant to be used for debugging purposes to test proper Tree/Leaf Node initialization
        /// @param room_pool A pool of different rooms, each with a unique weight value, used to populate the dungeon room
        /// @param seed Set int value to determine room generation layout
        BSP(const cse498::WeightedSet<std::string>& room_pool, 
			uint64_t seed,
			const std::string& file_path = std::string(DUNGEON_ROOMS_DIR) + "/Dungeon_")  
            : m_room_holder(room_pool, file_path)
        {
            m_rng.SetSeed(seed);
            insert_split(m_iterations);
            PostOrderDFS();
        }

        ////////////////////////////////////
        //    BSP Tree management
        ///////////////////////////////////

        /// @brief Creates a split in the grid to populate with a room
        /// @param iter number of iterations desired in the grid 
        /// @return integer value of a BSPNode's position in its container
        int insert_split(int iter) {
            BSPNode root_node = BSPNode{-1, -1, 0 , 0, m_width, m_height, std::string("Container")}; // creation of root node

            return insert_split(root_node, iter);
        }

        /// @brief Grabs all the leaf nodes of the BSP_Tree using Post-Order DFS, for map generation
        /// @param node root_node 
        void PostOrderDFS(BSPNode &node) {
            if (node.left_child == -1 && node.right_child == -1) {
                //Populating node with room and coordinate offset info
                m_room_holder.SetCurrentRoom();
                node.vector_room = m_room_holder.GetCurrentRoom();

                m_leaf_nodes.push_back(node);
                return;
            }

            PostOrderDFS(m_BSP_tree[node.left_child]);
            PostOrderDFS(m_BSP_tree[node.right_child]);
        }

        /// @brief Grabs all the leaf nodes of the BSP_Tree using Post-Order DFS for map generation
        void PostOrderDFS() {

            assert(m_BSP_tree.size() != 0 && "PostOrderDFS not initialized!");
            PostOrderDFS(m_BSP_tree[0]);
        }

        void ClearState() { 
            m_BSP_tree.clear();
            m_leaf_nodes.clear();
        }

        void RepopulateTree() { 
            insert_split(m_iterations); //Creates BSP Tree
            PostOrderDFS(); //Grabs all the generated room slots from the tree
        }

        /**
         * @brief Regenerates the BSP Tree incase any modifications to width/height/properties after its creation are made in order to 
         * ensure the object's state is up to date when new dungeons are later created
         */
        void RegenerateObjectState() {
            ClearState();
            RepopulateTree();
        }

        ////////////////////////////////////
        //    BSP Tree Debug Info
        ///////////////////////////////////
        
        /// @brief Generates Dungeon Map World outline of solely the splits from the BSP Tree without the rooms populated in them 
        /// @attention GenerateTileMap only shows the outline of the grid-map and the splits/paritions of the grid, it does not show the rooms
        void GenerateTileMap() {
            std::vector<std::string> grid(m_height, std::string(m_width, ' ')); //Copy of grid
            
            for (const auto& i : m_leaf_nodes) {
                auto copy_x = i.x;
                auto copy_y = i.y;

                for (int y = 0; y < i.height; y++) {
                    int grid_y = y + copy_y;

                    for (int x = 0; x < i.width; x++) { 
                        int grid_x = x + copy_x;
                        if (y == 0 || y == i.height - 1 || x == 0 || x == i.width - 1){
                            grid[grid_y][grid_x] = '.';
                        }
                            
                    }
                }
            }

        }

        /// @brief Simple parser that'll output the contents of the BSP Vector. 
        /// @attention Names of the same number are of the same depth level (name2 and name2 are the same level)
        void TreeParser() const {
            for (auto const& i : m_BSP_tree) {
                std::cout << "------------" << std::endl;
                std::cout << "Name " << i.name << std::endl;
                std::cout << "x " << i.x << std::endl;
                std::cout << "y " << i.y << std::endl;
                std::cout << "width " << i.width << std::endl;
                std::cout << "height " << i.height << std::endl;
                std::cout << "left_child, [Index]: " << i.left_child << std::endl;
                std::cout << "right_child, [Index]: " << i.right_child << std::endl;

                for (auto const& j : i.vector_room) {
                    std::cout << j << '\n';
                }
            }
        }
 
        /// @brief Sets RNG seed primarily for test case purposes
        /// @param integer seed value we want to set the RNG object too
        void SetRngSeed(uint64_t integer) {
            m_rng.SetSeed(integer);
        }

        uint64_t GetRngSeed() { 
            return m_rng.GetSeed();
        }


        ////////////////////////////////////
        //    Getters and Setters
        ///////////////////////////////////

        /// @brief grabs height of the grid map
        /// @return HEIGHT value
        [[nodiscard]] int GetHeight() {
            return m_height;
        }

        /// @brief grabs width of the rid map
        /// @return WIDTH value
        [[nodiscard]] int GetWidth() {
            return m_width;
        }   

        /// @brief updates the HEIGHT value for the grid map
        /// @param height 
        void SetHeight(int height) {
            m_height = height;
        }

        /// @brief updates the WIDTH value for the grid map
        /// @param width 
        void SetWidth(int width) {
            m_width = width;
        }
        
        /// @brief Sets the number of splits done to the BSP Tree
        /// @param iter 
        void SetIterations (int iter) {
            m_iterations = iter;
        }

        /// @brief Grabs the number of iterations done in splitting the BSP_Tree
        /// @return number of iterations we want to split the rooms with
        [[nodiscard]] int GetIterations() const { 
            return m_iterations;
        }

        /// @brief Returns the reference to BSP_Tree's leaf nodes
        [[nodiscard]] std::vector<BSPNode>& GetLeafNodes() {
            return m_leaf_nodes;
        }

        /// @brief Returns the reference to entirety of the created BSP_Tree
        [[nodiscard]] std::vector<BSPNode>& GetBSPTree() {
            return m_BSP_tree;
        }


    private:

        /// @brief Continuously populates the BSP tree until iter base case reaches 0
        /// @param iter number of iterations to split
        [[nodiscard]] int insert_split(BSPNode node, int iter) {
            int node_index = m_BSP_tree.size();
            m_BSP_tree.push_back(node);

            if (iter != 0) {

                auto split = random_split(node, iter); //Splits the grid into nodes


                //Prevent UE
                if(!split) {
                    return node_index;
                }

                auto [left_split, right_split] = *split;

                //std::cout << "left node iter: " << iter << " " << std::endl;
                auto left_node = insert_split(left_split, iter - 1);
                //std::cout << "-------------" << std::endl;
                //std::cout << "right node iter: " << iter << " " << std::endl;
                auto right_node = insert_split(right_split, iter -1);

                //connecting the nodes in the array-based tree based on index position
                m_BSP_tree[node_index].left_child = left_node;
                m_BSP_tree[node_index].right_child = right_node;

            }
            return node_index;
        }

        /// @brief Given an inputted struct root Node, splits the width/height randomly into  
        /// @param node 
        /// @return returns a tuple pair of Nodes, other returning nullopt if threshold width/height not met
        [[nodiscard]] std::optional<std::tuple<BSPNode, BSPNode>> random_split(BSPNode &node, int iter) {

            bool split_width = true;
            bool split_height = true;

            //If the width or the height of the partition do not meet the minimum threshold, stop the split
            // std::println("This is width: {}", node.width);
            // std::println("This is height: {}", node.height);
            if (node.width < m_threshold_width_value * 2) { split_width = false; }
            if (node.height < m_threshold_height_value * 2) { split_height = false; }

            // std::cout << split_width << " " << split_height << std::endl;


            if (!split_width && !split_height) {
                return std::nullopt;
            }

            BSPNode left_split, right_split;
            bool directional_split;

            if (split_width && split_height) {
                directional_split = (m_rng.GetValue(0,1).value() == 0);
            }
            else {
                directional_split = split_width;
            }
            
            if (directional_split) {
                ///Left split
                auto width_distributor = m_rng.GetValue(m_threshold_width_value, node.width - m_threshold_width_value).value();
                const int stored_width = width_distributor; 

                left_split = {
                    -1, -1,                 // left right child 
                    node.x, node.y,         // x-y coordinate
                    stored_width,           // width
                    node.height,            // height
                    std::to_string(iter)    //tree-depth (descending from iter) name
                };
                
                ///Right split
                right_split = {
                    -1, -1,                             // left right child 
                    node.x + left_split.width, node.y,  // x-y coordinate
                    node.width - left_split.width,      // width
                    node.height,                        // height
                    std::to_string(iter)                //tree-depth (descending from iter) name
                };
            }

            else {
                auto height_distributor = m_rng.GetValue(m_threshold_height_value, node.height - m_threshold_height_value).value();
                const int stored_height = height_distributor; 

                ///top split
                left_split = {
                    -1, -1,                 // left right child 
                    node.x, node.y,         // x-y coordinate
                    node.width,             // width
                    stored_height,          // height
                    std::to_string(iter)    //tree-depth (descending from iter) name
                };
                
                ///bottom split
                right_split = {
                    -1, -1,                             // left right child 
                    node.x, node.y + left_split.height, // x-y coordinate
                    node.width,                         // width
                    node.height - left_split.height,    // height
                    std::to_string(iter)                //tree-depth (descending from iter) name
                };
            }


            return std::make_tuple(left_split, right_split);

        }   
    };
}