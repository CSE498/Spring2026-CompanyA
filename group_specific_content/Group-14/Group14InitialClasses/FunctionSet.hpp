/**
 * FunctionSet: A container that holds a collection of functions, all with the exact same signature.
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

namespace cse498 {

  template <typename Signature>
  class FunctionSet;

  template <typename... Args>
  class FunctionSet<void(Args...)> {
  public:
    using FunctionID = std::uint64_t;
    using FunctionType = std::function<void(Args...)>;

    // Add a function and return its ID.
    FunctionID AddFunction(FunctionType fn) {
      const FunctionID id = next_id++;
      functions.push_back(Entry{id, std::move(fn)});
      return id;
    }

    // Remove a function by ID. Returns false if the ID is not found.
    bool RemoveFunction(FunctionID id) {
      const std::size_t old_size = functions.size();
      functions.erase(
          std::remove_if(functions.begin(), functions.end(),
                         [id](const Entry& e) { return e.id == id; }),
          functions.end());
      return functions.size() != old_size;
    }

    // Remove all functions.
    void Clear() { functions.clear(); }

    // How many functions are stored.
    std::size_t Size() const { return functions.size(); }

    // Call every stored function using the same arguments, one at a time.
    // If one throws, catch it and continue calling the rest.
    void CallAll(Args... args) const {
      std::size_t failures = 0;

      for (const Entry& entry : functions) {
        try {
          entry.fn(args...);
        } catch (...) {
          ++failures;
        }
      }

      last_call_failures = failures;
    }

    // Number of functions that threw during the most recent CallAll.
    std::size_t LastCallFailureCount() const { return last_call_failures; }

  private:
    struct Entry {
      FunctionID id;
      FunctionType fn;
    };

    std::vector<Entry> functions;  // stored functions in insertion order
    FunctionID next_id = 0;        // next ID to assign

    // Mutable so CallAll can stay const while tracking last-call status.
    mutable std::size_t last_call_failures = 0;
  };

}  // namespace cse498