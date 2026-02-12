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

size_t WIDTH = 300; //width of the grid (MAY NEED TO MAKE CONST)
size_t HEIGHT = 300; //hegiht of the grid (MAY NEED TO MAKE CONST)
size_t THRESHOLD_VALUE = 50; // min height/wdith value before cut-off
size_t ITERATIONS = 3; //number of cuts into the grid

namespace cse498 {
    /// @brief Holds basic information of the grids
    struct Node {
        
        int left_child = -1;
        int right_child = -1;

        int x, y, width, height;
        std::string name;


    };

    

    ///   
    class BSP {
    protected:
        std::vector<Node> BSP_Tree; //The entire tree
        std::vector<Node> leaf_nodes; //the split region tile rooms
        std::vector<std::string> grid{HEIGHT, std::string(WIDTH, '#')}; //map we're going to return for printing

    public: 

        BSP() = default;

        ////////////////////////////////////
        //    BSP Tree management
        ///////////////////////////////////

        /// @brief 
        /// @param iter 
        /// @return 
        int insert_split(int iter) {
            auto root_node{Node(-1, -1, 0 , 0, WIDTH, HEIGHT, "Container")}; // creation of root node

            return insert_split(root_node, iter);
        }

        /// @brief Rasterizing here
        void GenerateTileMap() {

        }

        /// @brief Simple parser that'll output the contents of the BST Vector 
        /// @return terminal output, no return value
        const void TreeParser() {
            for (auto const& i : BSP_Tree) {
                std::cout << "------------" << std::endl;
                std::cout << "Name " << i.name << std::endl;
                std::cout << "x " << i.x << std::endl;
                std::cout << "y " << i.y << std::endl;
                std::cout << "width " << i.width << std::endl;
                std::cout << "height " << i.height << std::endl;
                std::cout << "left_child, [Index]: " << i.left_child << std::endl;
                std::cout << "right_child, [Index]: " << i.right_child << std::endl;
            }
        }



        /// @brief Grabs all the paritioned regions of the map using Post-Order DFS, for map generation purposes
        /// @param node root_node 
        /// @return Vector of leaf nodes for tile generation
        const void PostOrderDFS(Node node) {
            if (node.left_child == -1 && node.right_child == -1) {
                leaf_nodes.push_back(node);
                return;
            }

            PostOrderDFS(BSP_Tree[node.left_child]);
            PostOrderDFS(BSP_Tree[node.right_child]);
            
        }

        const void PostOrderDFS() {
            if (BSP_Tree.size() == 0) {
                throw std::runtime_error("Tree has not been properly initialized! Have you called insert_split yet?");
            }
            PostOrderDFS(BSP_Tree[0]);
        }

        /// @brief Called when player agent will enter new level, layout and tile grid will be remade 
        /// @return 
        const void Update() {
            BSP_Tree.clear();
            leaf_nodes.clear();
            grid.assign(HEIGHT, std::string(WIDTH, '#'));

        }
        
        ////////////////////////////////////
        //    Getters and Setters
        ///////////////////////////////////

        /// @brief grabs height of the grid map
        /// @return HEIGHT value
        [[nodiscard]] const size_t GetHeight() {
            return HEIGHT;
        }

        /// @brief grabs width of the rid map
        /// @return WIDTH value
        [[nodiscard]] const size_t GetWidth() {
            return WIDTH;
        }   

        /// @brief updates the HEIGHT value for the grid map
        /// @param height 
        const void SetHeight(size_t height) {
            HEIGHT = height;
        }

        /// @brief updates the WIDTH value for the grid map
        /// @param width 
        const void SetWidth(size_t width) {
            WIDTH = width;
        }

        const void SetIterations (size_t iter) {
            ITERATIONS = iter;
        }

        const size_t GetIterations() { 
            return ITERATIONS;
        }

    private:
        /// @brief Continuously populates the BSP tree until iter base case reaches 0
        /// @param iter number of iterations to split
        int insert_split(Node node, int iter) {
            int node_index = BSP_Tree.size();
            BSP_Tree.push_back(node);

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
                BSP_Tree[node_index].left_child = left_node;
                BSP_Tree[node_index].right_child = right_node;

            }
            
            return node_index;
        

        }

        /// @brief Given an inputted struct root Node, splits the width/height randomly into  
        /// @param node 
        /// @return returns a tuple pair of Nodes, other returning nullopt if threshold width/height not met
        [[nodiscard]] std::optional<std::tuple<Node, Node>> random_split(Node &node, int& iter) {
            bool split_width = false;
            bool split_height = false;

            //If the width or the height of the partition do not meet the minimum threshold, stop the split
            if (node.width < THRESHOLD_VALUE * 2 || node.height < THRESHOLD_VALUE * 2) {
                return std::nullopt;
            }


            std::random_device rd;
            std::mt19937 mt(rd());
            std::uniform_int_distribution<int> width_distributor(THRESHOLD_VALUE, node.width - THRESHOLD_VALUE);
            std::uniform_int_distribution<int> height_distributor(THRESHOLD_VALUE, node.height - THRESHOLD_VALUE);
            std::uniform_int_distribution<int> directional_splitter(0,1);

            Node left_split, right_split;

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

            if (directional_splitter(mt) == 0) {    
                ///Left split
                left_split = {
                    -1, -1,                 // left right child 
                    node.x, node.y,         // x-y coordinate
                    stored_width,           // width
                    node.height,            // height
                    std::to_string(iter)    //tree-depth (descending from iter)
                };
                
                ///Right split
                right_split = {
                    -1, -1,                             // left right child 
                    node.x + left_split.width, node.y,  // x-y coordinate
                    node.width - left_split.width,      // width
                    node.height,                        // height
                    std::to_string(iter)                //tree-depth (descending from iter)
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
                    std::to_string(iter)    //tree-depth (descending from iter)
                };
                
                ///bottom split
                right_split = {
                    -1, -1,                             // left right child 
                    node.x, node.y + left_split.height, // x-y coordinate
                    node.width,                         // width
                    node.height - left_split.height,    // height
                    std::to_string(iter)                //tree-depth (descending from iter)
                };

            }

            return std::make_tuple(left_split, right_split);

        }

    };

}