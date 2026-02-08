/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief 
 * @note Status: PROPOSAL
 * Credit notes: This files makes use parts of the Array implementation of a Binary Tree described at:
 * https://www.geeksforgeeks.org/dsa/binary-tree-array-implementation/
 * Weight searching function adapted from simple array version described at:
 * https://stackoverflow.com/questions/1761626/weighted-random-numbers (top answer)
 **/

#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include  <unordered_map>

constexpr double TOL = 1e-9;

namespace cse498 {

	using std::string;
	using std::cout;
	using std::endl;

	template <typename T>
	class WeightedSet {
		private:
		//Storage should be relatively small assuming T will be a string or int (probable for ids)
		std::unordered_map<T, int> item_idx{};
		std::vector<T> items{};
		std::vector<double> weights{};
		std::vector<double> sum_tree{}; 

		void FixSum(int idx, double weight) {
			while (idx > 0){
				sum_tree[idx] += weight;
				idx = (idx - 1) / 2; //Travel up parents
			}
			sum_tree[0] += weight; //Fix the root
		}

		public:
		WeightedSet(const WeightedSet&) = delete;
    		
		WeightedSet& operator=(const WeightedSet&) = delete; 
			
		WeightedSet() = default;
			
		~WeightedSet() = default;

		void Insert(const T& id, double weight){
			if (weight <= 0.0) {
				throw std::invalid_argument(
					"weight must be greater than 0"
				);
			}
			if (item_idx.contains(id)) {
				throw std::invalid_argument("duplicate item");
			}

			int idx = static_cast<int>(sum_tree.size());
			item_idx[id] = idx;
			items.push_back(id);
			weights.push_back(weight);
			sum_tree.push_back(0.0);

			FixSum(idx, weight);
		}

		T Sample(double num){
			if (sum_tree.empty() || sum_tree[0] <= TOL){
				throw std::runtime_error("Cannot sample from an empty WeightedSet");
			}
			if (num < 0 || num > sum_tree[0] + TOL) {
				throw std::invalid_argument(
					"Sample number invalid"
				);
			}

			int idx = 0;
			int tree_size = static_cast<int>(sum_tree.size());

			while (true){
				int l_idx = 2*idx + 1;
				int r_idx = 2*idx + 2;

				double l_sum = 0.0;
				if (l_idx < tree_size){
					l_sum = sum_tree[l_idx]; 
				}

				//Travel down left subtree
				if (num <= l_sum + TOL){
					if (l_idx < tree_size){ //If left child exists
						idx = l_idx;
					}
					else{
						break; //found correct node
					}
				}
				else{
					//Check item at current index
					num -= l_sum;
					if (num <= weights[idx] + TOL){
						break; //We found the correct item
					}

					//Travel down right subtree
					num -= weights[idx];

					if (r_idx >= tree_size) { //Accounts for a tolerance error
						break;
					}
					idx = r_idx;
				}
			}
			return items[idx];
		}
	};

} // End of namespace cse498


