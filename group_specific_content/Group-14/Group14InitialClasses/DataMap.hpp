/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Maps strings to arbitrary data types. Caller is not required to perform casts. Types in the DataMap are
 * explicit at the call site. Any type mismatches are handled up front by DataMap.
 **/

#pragma once

#include <unordered_map> /// For basic map implementation
#include <string>
#include <any>           /// For storing any type into unordered_map
#include <typeindex>     /// For runtime type storage
#include <typeinfo>
#include <cassert>       /// For assertions

namespace cse498 {

	/// @struct Entry
	/// @brief Contains data for an entry in the DataMap
	struct Entry {
		std::type_index type;   /// Entry type
		std::any value;         /// Entry value
	};

	/// @class DataMap
	/// @brief Map strings to an arbitrary data type.
	class DataMap {
	private:
	    std::unordered_map<std::string, Entry> map;       /// Main map
	public:
	    /// Set a key value pair
	    template<typename T>
	    void Set(const std::string& key, const T& value) {
		    // Find key, value pair if it exists
		    auto it = map.find(key);
		    // No pair found, Add new association to map
		    if (it == map.end()) { map.emplace(key, Entry{ typeid(T), std::any(value) }); }
			    // Pair found
		    else {
			    // Make sure value being updated has the same type
			    assert((typeid(T) == it->second.type && "Attempting to change value of mismatched types!"));
			    // Update the value of the pair
			    it->second.value = value;
		    }
	    }
	    /// Get a value from a key
	    template<typename T>
	    T& Get(const std::string& key) {
		    // Look for pair
		    auto it = map.find(key);
		    // Ensure pair exists
		    assert((it != map.end() && "Key Value pair does not exist!"));
		    // Return the value of the found pair
		    assert((typeid(T) == it->second.type && "Type mismatch on Get!"));
		    return *std::any_cast<T>(&it->second.value);
	    }
	    /// Check to see if key is in the map
	    bool Has(const std::string& key) const { return map.find(key) != map.end(); }
	    /// Remove key from map. Returns bool if successful
	    bool Remove(const std::string& key) {
		    // Make sure key exists
		    if (!Has(key)) { return false; }
		    map.erase(key);
		    return true;
	    }
	    /// Clear out map of all data
	    void Clear() { map.clear(); }
	    /// Checks if the map is empty
	    bool Empty() { return map.empty(); }
	};
}
