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
#include <expected>
#include <vector>
#include <unordered_map>

namespace cse498 {

	template <typename T>
	class WeightedSet {
		private:
		static constexpr double WEIGHTED_SET_TOL = 1e-12;

		//Storage should be relatively small assuming T will be a string or int (probable for ids)
		std::unordered_map<T, size_t> item_idx{};
		std::vector<T> items{};
		std::vector<double> weights{};
		std::vector<double> sum_tree{}; 
		std::size_t set_size = 0;

		/**
		 * Propagates a weight change up the implicit tree.
		 *
		 * @param idx Index of the modified node.
		 * @param w_change Difference to apply to subtree sums.
		 */
		void FixSum(size_t idx, double w_change) {
			while (idx > 0){
				sum_tree[idx] += w_change;
				idx = (idx - 1) / 2; //Travel up parents
			}
			sum_tree[0] += w_change; //Fix the root
		}

		/**
		 * Normalizes very small weights to zero.
		 * 
		 * @param weight Weight value to normalize (modified in place).
		 */
		void FixTinyNum(double& weight){
			//Tiny weights below tolerance that are not 0 can cause strange sample outputs
			if (weight < WEIGHTED_SET_TOL) {
				weight = 0.0; //Tiny weights now treated as 0.0
			}
		}

		public:
		WeightedSet() = default;
			
		~WeightedSet() = default;

		/**
		 * Inserts a new item with the given weight.
		 *
		 * @param id Unique identifier for the item.
		 * @param weight Non-negative weight associated with the item.
		 *
		 * @throws std::invalid_argument if weight is negative or id already exists.
		 */
		std::expected<void, std::string> Insert(const T& id, double weight){
			if (weight < 0.0) {
				return std::unexpected("cse498::WeightedSet::Insert(): weight must be non-negative");
			}
			if (item_idx.contains(id)) {
				return std::unexpected("cse498::WeightedSet::Insert(): duplicate item");
			}

			FixTinyNum(weight);

			size_t idx = set_size;
			item_idx[id] = idx;
			items.push_back(id);
			weights.push_back(weight);
			sum_tree.push_back(0.0);
			++set_size;

			FixSum(idx, weight);

			return {};
		}

		/**
		 * Updates the weight of an existing item.
		 *
		 * @param id Identifier of the item to update.
		 * @param weight New non-negative weight.
		 *
		 * @throws std::invalid_argument if weight is negative or item does not exist.
		 */
		std::expected<void, std::string> Update(const T& id, double weight) {
			if (weight < 0.0) {
				return std::unexpected("cse498::WeightedSet::Update(): weight must be non-negative");
			}
			if (!item_idx.contains(id)) {
				return std::unexpected("cse498::WeightedSet::Update(): item to update does not exist");
			}

			FixTinyNum(weight);

			size_t idx = item_idx[id];
			double change = weight - weights[idx];
			weights[idx] = weight;
			FixSum(idx, change);

			return {};
		}

		/**
		 * Samples an item according to its weight.
		 *
		 * Given a value in the range [0, total_weight], returns the item whose
		 * cumulative weight interval contains the value.
		 *
		 * This function performs a traversal over a vector based sum tree
		 *
		 * @param num Sample value
		 *
		 * @return The sampled item.
		 *
		 * @throws std::runtime_error if the set is empty.
		 * @throws std::invalid_argument if num is outside the valid range.
		 */
		std::expected<T, std::string> Sample(double num) const {
			if (sum_tree.empty() || sum_tree[0] <= WEIGHTED_SET_TOL){
				return std::unexpected("cse498::WeightedSet::Sample(): Cannot sample from an empty WeightedSet");
			}
			if (num < 0 || num > sum_tree[0] + WEIGHTED_SET_TOL) {
				return std::unexpected("cse498::WeightedSet::Sample(): Sample number invalid");
			}

			//lower and upper endpoints of the full range of values. Ex; [0.0, 5.1]
			double outer_lo = 0.0; //lower value in outer interval
			double outer_up = sum_tree[0]; //upper value in outer interval
			size_t idx = 0;

			while(true){
				assert(num >= outer_lo - WEIGHTED_SET_TOL && num <= outer_up + WEIGHTED_SET_TOL);

				size_t left_idx = 2*idx + 1;
    			size_t right_idx = 2*idx + 2;

				double left_sum = (left_idx < set_size) ? sum_tree[left_idx] : 0.0;

				//interval of "node" at current index 
				double inner_lo = outer_lo + left_sum; //Lower value in node's interval
				double inner_up = inner_lo + weights[idx]; //Upper value in node's interval

				//Case 1: number in the left subtree interval
				if (num <= inner_lo + WEIGHTED_SET_TOL 
					&& left_idx < set_size
					&& left_sum > WEIGHTED_SET_TOL) {

					outer_up = inner_lo;
					idx = left_idx;
					continue;
				}

				//Case 2: num in current inner inverval (a, b]
				if (num > inner_lo - WEIGHTED_SET_TOL 
					&& num <= inner_up + WEIGHTED_SET_TOL 
					&& weights[idx] > WEIGHTED_SET_TOL) {
						
					break; //Found corresponding item
				}

				//Case 3: number in the right subtree interval
				if (right_idx >= set_size) { //for safety - this shouldnt run (Case 1 should have)
					assert(right_idx < set_size); 
					break; //return this node since there is no left or right
				}
				outer_lo = inner_up;
				idx = right_idx;
			}
			return items[idx];
		}

		size_t GetSize() const { return set_size; }
		double GetItemSum(const T& item) const { return sum_tree.at(item_idx.at(item)); }
		double GetWeight(const T& item) const { return weights.at(item_idx.at(item)); }
		double GetTotalWeight() const { return sum_tree.at(0); }
	};

} // End of namespace cse498