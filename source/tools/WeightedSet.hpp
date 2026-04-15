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
#include <string>
#include <concepts>
#include <functional>

namespace cse498 {

	/**
	 * @concept Hashable
	 * @brief A type that can be hashed with std::hash.
	 *
	 * Requires that std::hash<T> is defined and callable for objects of type T,
	 * and that the hash result is convertible to std::size_t.
	 *
	 * @param T The type being checked.
	 */
	template<typename T>
	concept Hashable = requires(T a)
	{
		{ std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
	};

	/**
	 * @concept UnorderedMapKey
	 * @brief A type that can be used as a key in std::unordered_map.
	 *
	 * Requires that T is hashable and equality comparable.
	 *
	 * @tparam T The type being checked.
	 */
	template <typename T>
	concept UnorderedMapKey = Hashable<T> && std::equality_comparable<T>;

	/**
	 * @class WeightedSet
	 * @brief Stores items with weights and supports efficient weighted sampling.
	 *
	 * This class maintains a collection of items of type `T`, each with a non-negative weight.
	 * It allows insertion, weight updates, and sampling proportional to weights.
	 *
	 * Internally, it uses a vector-based binary sum tree for O(log n) updates and sampling,
	 * along with a unordered map for fast item lookup.
	 *
	 * @param T Type of stored items, must satisfy `UnorderedMapKey`, meaning it
	 * must be hashable and equality comparable
	 */
	template <UnorderedMapKey T>
	class WeightedSet {
		private:

		// Small tolerance used to handle floating-point precision errors in weight comparisons
		static constexpr double WEIGHTED_SET_TOL = 1e-12;

		//Storage should be relatively small assuming T will be a string or int (probable for ids)
		std::unordered_map<T, std::size_t> m_item_idx{};
		std::vector<T> m_items{};
		std::vector<double> m_weights{};
		std::vector<double> m_sum_tree{}; 
		std::size_t m_set_size = 0;

		/**
		 * Propagates a weight change up the implicit tree.
		 *
		 * @param idx Index of the modified node.
		 * @param w_change Difference to apply to subtree sums.
		 */
		void PropagateWeightChange(std::size_t idx, double w_change) {
			while (idx > 0){
				m_sum_tree[idx] += w_change;
				idx = (idx - 1) / 2; //Travel up parents
			}
			m_sum_tree[0] += w_change; //Fix the root
		}

		/**
		 * Changes all very small weights to be zero.
		 * 
		 * @param weight Weight value to normalize (modified in place).
		 */
		void NormalizeTinyWeight(double& weight){
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
		 * @return std::expected<void, std::string>
 		 *         - On success: contains no value.
 		 *         - On failure: contains an error message describing the issue.
		 *
		 * @note Returns an error if:
		 *       - weight is negative
		 *       - id already exists in the set
		 */
		[[nodiscard]] std::expected<void, std::string> Insert(const T& id, double weight){
			if (weight < 0.0) {
				return std::unexpected("cse498::WeightedSet::Insert(): weight must be non-negative");
			}
			if (m_item_idx.contains(id)) {
				return std::unexpected("cse498::WeightedSet::Insert(): duplicate item");
			}

			//Treat numbers below tolerance as 0
			NormalizeTinyWeight(weight);

			std::size_t idx = m_set_size;
			m_item_idx[id] = idx;
			m_items.push_back(id);
			m_weights.push_back(weight);
			m_sum_tree.push_back(0.0);
			++m_set_size;

			PropagateWeightChange(idx, weight);

			return {};
		}

		/**
		 * Updates the weight of an existing item.
		 *
		 * @param id Identifier of the item to update.
		 * @param weight New non-negative weight.
		 *
		 * @return std::expected<void, std::string>
 *         - On success: contains no value.
		 *         - On failure: contains an error message describing the issue.
		 *
		 * @note Returns an error if:
		 *       - weight is negative
		 *       - item does not exist in the set
		 */
		[[nodiscard]] std::expected<void, std::string> Update(const T& id, double weight) {
			if (weight < 0.0) {
				return std::unexpected("cse498::WeightedSet::Update(): weight must be non-negative");
			}

			auto iter = m_item_idx.find(id);
		
			if (iter == m_item_idx.end()) {
				return std::unexpected("cse498::WeightedSet::Update(): item to update does not exist");
			}

			//Treat numbers below tolerance as 0
			NormalizeTinyWeight(weight);

			std::size_t idx = iter->second;
			assert(idx < m_weights.size()); //If item exists in map there should already be an associated weight
			double change = weight - m_weights[idx];
			m_weights[idx] = weight;
			PropagateWeightChange(idx, change);

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
		 * @param sample_val Value to sample for
		 *
		 * @return The sampled item.
		 *
		 * @note The set can not be empty.
		 * @note sample_val is inside  the valid range [0, total_weight].
		 */
		[[nodiscard]] std::expected<T, std::string> Sample(double sample_val) const {
			if (m_sum_tree.empty() || m_sum_tree[0] <= WEIGHTED_SET_TOL){
				return std::unexpected("cse498::WeightedSet::Sample(): Cannot sample from an empty WeightedSet");
			}
			if (sample_val < 0 || sample_val > m_sum_tree[0] + WEIGHTED_SET_TOL) {
				return std::unexpected("cse498::WeightedSet::Sample(): Sample number invalid");
			}

			//lower and upper endpoints of the full range of values. Ex; [0.0, 5.1]
			double outer_lo = 0.0; //lower value in outer interval
			double outer_up = m_sum_tree[0]; //upper value in outer interval
			std::size_t idx = 0;

			while(true){
				assert(sample_val >= outer_lo - WEIGHTED_SET_TOL && sample_val <= outer_up + WEIGHTED_SET_TOL);

				std::size_t left_idx = 2*idx + 1;
    			std::size_t right_idx = 2*idx + 2;

				double left_sum = (left_idx < m_set_size) ? m_sum_tree[left_idx] : 0.0;

				//interval of "node" at current index 
				double inner_lo = outer_lo + left_sum; //Lower value in node's interval
				double inner_up = inner_lo + m_weights[idx]; //Upper value in node's interval

				//Case 1: number in the left subtree interval
				if (sample_val <= inner_lo + WEIGHTED_SET_TOL 
					&& left_idx < m_set_size
					&& left_sum > WEIGHTED_SET_TOL) {

					outer_up = inner_lo;
					idx = left_idx;
					continue;
				}

				//Case 2: sample_val in current inner inverval (a, b]
				if (sample_val > inner_lo - WEIGHTED_SET_TOL 
					&& sample_val <= inner_up + WEIGHTED_SET_TOL 
					&& m_weights[idx] > WEIGHTED_SET_TOL) {
						
					break; //Found corresponding item
				}

				//Case 3: number in the right subtree interval
				if (right_idx >= m_set_size) { //for safety - this shouldnt run (Case 1 should have)
					assert(right_idx < m_set_size); 
					break; //return this node since there is no left or right
				}
				outer_lo = inner_up;
				idx = right_idx;
			}
			return m_items[idx];
		}

		/**
		 * Gets the cumulative sum for an item.
		 * @param item Item identifier.
		 * @return Sum if found, otherwise std::unexpected.
		 */
		[[nodiscard]] std::expected<double, std::string> GetItemSum(const T& item) const {
			auto iter = m_item_idx.find(item);
			if (iter == m_item_idx.end()) {
				return std::unexpected("cse498::WeightedSet::GetItemSum(): item not found");
			}
			assert(iter->second < m_sum_tree.size());

			return m_sum_tree[iter->second];
		}

		/**
		 * Gets the weight of an item.
		 * @param item Item identifier.
		 * @return Weight if found, otherwise std::unexpected.
		 */
		[[nodiscard]] std::expected<double, std::string> GetWeight(const T& item) const {
			auto iter = m_item_idx.find(item);
			if (iter == m_item_idx.end()) {
				return std::unexpected("cse498::WeightedSet::GetWeight(): item not found");
			}

			assert(iter->second < m_weights.size());

			return m_weights[iter->second];
		}
		
		/**
		 * Gets the total weight of all items.
		 * @return Total weight (0.0 if empty).
		 */
		[[nodiscard]] double GetTotalWeight() const {
			return m_sum_tree.empty() ? 0.0 : m_sum_tree.at(0);
		}

		/**
		 * Gets the number of items.
		 * @return Current size.
		 */
		[[nodiscard]] std::size_t GetSize() const { return m_set_size; }
	};

} // End of namespace cse498