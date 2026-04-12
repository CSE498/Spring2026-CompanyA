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
#include "RoomHolder.hpp"



namespace cse498 {
    /// @brief Holds basic information of the grids
    struct BSPNode {
        
        int left_child = -1; //Left node of the tree
        int right_child = -1; //right node of the tree
        int x, y, width, height; // (x,y) - (origin point of the grid (top-left corner))
                                 // (width, height) - dimension of the grid cut (lxw)
        std::string name; //Name of the node for debugging purposes
        std::string split;
        std::vector<std::string> vector_room{}; //Stores a certain dungeon room
        

        BSPNode() = default;
        BSPNode(int l, int r, int x, int y, int width, int height, std::string name) 
           : left_child(l), right_child(r), x(x), y(y), width(width), height(height), name(name) {}

    };
    

    ///   
    class BSP {
    protected:
        RoomHolder mRoomHolder;
        std::vector<BSPNode> mBSP_Tree; //The entire tree
        std::vector<BSPNode> mLeafNodes; //the split region tile rooms

        int mWidth = 150; //width of the grid (MAY NEED TO MAKE CONST)
        int mHeight = 150; //hegiht of the grid (MAY NEED TO MAKE CONST)
        int mThresholdValue = 30; // min height/wdith value before cut-off
        int mIterations = 5; //number of splits into the grid


    public: 


        /// @brief Constructor call creates the BSP Tree from the get-go, meaning that BSP_Tree and its leaf nodes are already populated 
        BSP() : mRoomHolder() { 
            insert_split(mIterations);
            PostOrderDFS();
        }

        ////////////////////////////////////
        //    BSP Tree management
        ///////////////////////////////////

        /// @brief 
        /// @param iter number of iterations desired in the grid 
        /// @return 
        int insert_split(int iter) {
            BSPNode root_node = BSPNode{-1, -1, 0 , 0, mWidth, mHeight, std::string("Container")}; // creation of root node

            return insert_split(root_node, iter);
        }


        /// @brief Grabs all the leaf nodes of the BSP_Tree using Post-Order DFS, for map generation
        /// @param node root_node 
        void PostOrderDFS(BSPNode &node) {
            if (node.left_child == -1 && node.right_child == -1) {
                mRoomHolder.SetCurrentRoom();
                node.vector_room = mRoomHolder.GetCurrentRoom();

                mLeafNodes.push_back(node);
                return;
            }

            PostOrderDFS(mBSP_Tree[node.left_child]);
            PostOrderDFS(mBSP_Tree[node.right_child]);
            
        }

        /// @brief Grabs all the leaf nodes of the BSP_Tree using Post-Order DFS for map generation
        void PostOrderDFS() {

            assert(mBSP_Tree.size() != 0 && "PostOrderDFS not initialized!");
            PostOrderDFS(mBSP_Tree[0]);
        }

        /// @brief Called when player agent will enter new level, layout and tile grid will be remade 

        void Update() {
            mBSP_Tree.clear();
            mLeafNodes.clear();

        }

        ////////////////////////////////////
        //    BSP Tree Debug Info
        ///////////////////////////////////
        
        /// @brief Generates Grid map of the solely the splits from the BSP Tree without the rooms in them 
        void GenerateTileMap() {
            std::vector<std::string> grid(mHeight, std::string(mWidth, ' ')); //Copy of grid
            
            for (const auto& i : mLeafNodes) {
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
            
            for (auto i : grid) {
                std::cout << i << '\n';
            }
        }



        /// @brief Simple parser that'll output the contents of the BSP Vector. 
        /// @attention Names of the same number are of the same depth level (name2 and name2 are the same level)
        /// @return terminal output, no return value
        void TreeParser() const {
            for (auto const& i : mBSP_Tree) {
                std::cout << "------------" << std::endl;
                std::cout << "Name " << i.name << std::endl;
                std::cout << "x " << i.x << std::endl;
                std::cout << "y " << i.y << std::endl;
                std::cout << "width " << i.width << std::endl;
                std::cout << "height " << i.height << std::endl;
                std::cout << "split " << i.split << std::endl;
                std::cout << "left_child, [Index]: " << i.left_child << std::endl;
                std::cout << "right_child, [Index]: " << i.right_child << std::endl;

                for (auto const& j : i.vector_room) {
                    std::cout << j << '\n';
                }
            }
        }


        
        ////////////////////////////////////
        //    Getters and Setters
        ///////////////////////////////////

        /// @brief grabs height of the grid map
        /// @return HEIGHT value
        [[nodiscard]] size_t GetHeight() {
            return mHeight;
        }

        /// @brief grabs width of the rid map
        /// @return WIDTH value
        [[nodiscard]] size_t GetWidth() {
            return mWidth;
        }   

        /// @brief updates the HEIGHT value for the grid map
        /// @param height 
        void SetHeight(size_t height) {
            mHeight = height;
        }

        /// @brief updates the WIDTH value for the grid map
        /// @param width 
        void SetWidth(size_t width) {
            mWidth = width;
        }
        
        /// @brief 
        /// @param iter 
        void SetIterations (size_t iter) {
            mIterations = iter;
        }

        //Don't know if we'll actually need this, leaving this here for now just in-case
        // void SetRoom (std::vector<std::string>) {
            
        // }

        /// @brief Grabs the number of iterations done in splitting the BSP_Tree
        /// @return number of iterations we want to split the rooms with
        [[nodiscard]] size_t GetIterations() const { 
            return mIterations;
        }

        //Returns the BSP_Tree's leaf nodes
        [[nodiscard]] std::vector<BSPNode> GetLeafNodes() const {
            return mLeafNodes;
        }

        //Returns the entirety of the created BSP_Tree
        [[nodiscard]] std::vector<BSPNode> GetBSPTree() const {
            return mBSP_Tree;
        }


    private:

        /// @brief Continuously populates the BSP tree until iter base case reaches 0
        /// @param iter number of iterations to split
        [[nodiscard]] int insert_split(BSPNode node, int iter) {
            int node_index = mBSP_Tree.size();
            mBSP_Tree.push_back(node);

            if (iter != 0) {

                auto split = random_split(node, iter); //Splits the grid into nodes


                //Prevent UE
                if(!split) {
                    return node_index;
                }

                auto [left_split, right_split] = *split;

                auto left_node = insert_split(left_split, iter - 1);
                auto right_node = insert_split(right_split, iter -1);

                //connecting the nodes in the array-based tree based on index position
                mBSP_Tree[node_index].left_child = left_node;
                mBSP_Tree[node_index].right_child = right_node;

            }

            
            return node_index;
        

        }

        /// @brief Given an inputted struct root Node, splits the width/height randomly into  
        /// @param node 
        /// @return returns a tuple pair of Nodes, other returning nullopt if threshold width/height not met
        [[nodiscard]] std::optional<std::tuple<BSPNode, BSPNode>> random_split(BSPNode &node, int& iter) {

            //bool split_width = true;
            //bool split_height = true;
            //If the width or the height of the partition do not meet the minimum threshold, stop the split
            if (node.width < mThresholdValue * 2 || node.height < mThresholdValue * 2) {
                return std::nullopt;
            }


            std::random_device rd;
            std::mt19937 mt(rd());
            std::uniform_int_distribution<int> width_distributor(mThresholdValue, node.width - mThresholdValue);
            std::uniform_int_distribution<int> height_distributor(mThresholdValue, node.height - mThresholdValue);
            std::uniform_int_distribution<int> directional_splitter(0,10);

            BSPNode left_split, right_split;

            //storing random values to ensure stable numbers throughout all calls
            // std::cout << "NODE WIDTH/HEIGHT: " << node.width << " " << node.height << std::endl; 
            // std::cout << "THRESHOLD: " << THRESHOLD_VALUE << std::endl;
            // std::cout << "DIFFERENCE: " << node.width - THRESHOLD_VALUE << std::endl;

            const int stored_width = width_distributor(mt); 
            const int stored_height = height_distributor(mt); 

            // std::cout << stored_width << " " << stored_height << std::endl;
            // std::cout << "------------------" << std::endl;
            // std::cout << node.x << std::endl;
            // std::cout << node.y << std::endl;
            // std::cout << node.width << std::endl;
            // std::cout << node.height << std::endl;
            // std::cout << node.left_child << std::endl;
            // std::cout << node.right_child << std::endl;

            ///Vertical split

            auto testing = directional_splitter(mt);

            if (testing <= 5) {    
                ///Left split
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

            ///Horizontal split
            else {
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