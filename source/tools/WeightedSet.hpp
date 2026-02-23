/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief 
 * @note Status: PROPOSAL
 * Credit notes: This files makes use parts of the Array implementation of a Binary Tree described at:
 * https://www.geeksforgeeks.org/dsa/binary-tree-array-implementation/
 * Weight searching function adapted from simple array version described at:
 * https://stackoverflow.com/questions/1761626/weighted-random-numbers (top answer)
 **/

#pragma once

#include <cassert>
#include <stdexcept>
#include <vector>
#include <unordered_map>

namespace cse498 {

	constexpr double WEIGHTED_SET_TOL = 1e-12;

	template <typename T>
	class WeightedSet {
		private:
		//Storage should be relatively small assuming T will be a string or int (probable for ids)
		std::unordered_map<T, int> item_idx{};
		std::vector<T> items{};
		std::vector<double> weights{};
		std::vector<double> sum_tree{}; 
		std::size_t set_size = 0;

		void FixSum(int idx, double w_change) {
			while (idx > 0){
				sum_tree[idx] += w_change;
				idx = (idx - 1) / 2; //Travel up parents
			}
			sum_tree[0] += w_change; //Fix the root
		}

		void FixTinyNum(double& weight){
			//Tiny weights below tolerance that are not 0 can cause strange sample outputs
			if (weight < WEIGHTED_SET_TOL) {
				weight = 0.0; //Tiny weights now treated as 0.0
			}
		}

		public:
		WeightedSet(const WeightedSet&) = delete;
    		
		WeightedSet& operator=(const WeightedSet&) = delete; 
			
		WeightedSet() = default;
			
		~WeightedSet() = default;

		void Insert(const T& id, double weight){
			if (weight < 0.0) {
				throw std::invalid_argument("cse498::WeightedSet::Insert(): weight must be non-negative");
			}
			if (item_idx.contains(id)) {
				throw std::invalid_argument("cse498::WeightedSet::Insert(): duplicate item");
			}

			FixTinyNum(weight);

			int idx = static_cast<int>(set_size);
			item_idx[id] = idx;
			items.push_back(id);
			weights.push_back(weight);
			sum_tree.push_back(0.0);
			++set_size;

			FixSum(idx, weight);
		}

		void Update(const T& id, double weight) {
			if (weight < 0.0) {
				throw std::invalid_argument("cse498::WeightedSet::Update(): weight must be non-negative");
			}

			if (!item_idx.contains(id)) {
				throw std::invalid_argument("cse498::WeightedSet::Update(): item to update does not exist");
			}

			FixTinyNum(weight);

			int idx = item_idx[id];
			double change = weight - weights[idx];
			weights[idx] = weight;
			FixSum(idx, change);
		}

		T Sample(double num) const {
			if (sum_tree.empty() || sum_tree[0] <= WEIGHTED_SET_TOL){
				throw std::runtime_error("cse498::WeightedSet::Sample(): Cannot sample from an empty WeightedSet");
			}
			if (num < 0 || num > sum_tree[0] + WEIGHTED_SET_TOL) {
				throw std::invalid_argument("cse498::WeightedSet::Sample(): Sample number invalid");
			}

			//lower and upper endpoints of the full range of values. Ex; [0.0, 5.1]
			double outer_lo = 0.0;
			double outer_up = sum_tree[0];
			int idx = 0;
			int tree_size = static_cast<int>(sum_tree.size());

			while(true){
				assert(num >= outer_lo - WEIGHTED_SET_TOL && num <= outer_up + WEIGHTED_SET_TOL);

				int l_idx = 2*idx + 1;
    			int r_idx = 2*idx + 2;

				double left_sum = (l_idx < tree_size) ? sum_tree[l_idx] : 0.0;

				//interval of "node" at current index 
				double inner_lo = outer_lo + left_sum;
				double inner_up = inner_lo + weights[idx];

				//Case 1: number in the left subtree interval
				if (num <= inner_lo + WEIGHTED_SET_TOL 
					&& l_idx < tree_size 
					&& left_sum > WEIGHTED_SET_TOL) {

					outer_up = inner_lo;
					idx = l_idx;
					continue;
				}

				//Case 2: num in current inverval (a, b]
				if (num > inner_lo - WEIGHTED_SET_TOL 
					&& num <= inner_up + WEIGHTED_SET_TOL 
					&& weights[idx] > WEIGHTED_SET_TOL) {
						
					break; //Found corresponding item
				}

				//Case 3: number in the right subtree interval
				if (r_idx >= tree_size) { //for safety - this shouldnt run (Case 1 should have)
					assert(r_idx < tree_size); 
					break; //return this node since there is no left or right
				}
				outer_lo = inner_up;
				idx = r_idx;
			}
			return items[idx];
		}

		int GetSize() const { return set_size; }
		double GetItemSum(const T& item) const { return sum_tree.at(item_idx.at(item)); }
		double GetWeight(const T& item) const { return weights.at(item_idx.at(item)); }
	};

} // End of namespace cse498