/**
 * FunctionSet: A container that holds a collection of functions, all with the exact same signature.
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <ranges>
#include <type_traits>
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

    struct Entry {
      FunctionID id;
      FunctionType fn;
    };

    // Add any callable compatible with void(Args...)
    template <typename F>
      requires std::is_invocable_r_v<void, F&, Args...>
    FunctionID AddFunction(F&& fn) {
      const FunctionID id = mNextID++;
      mFunctions.push_back(Entry{id, FunctionType(std::forward<F>(fn))});
      return id;
    }

    // Remove a function by ID. Returns false if the ID is not found.
    bool RemoveFunction(FunctionID id) {
      const std::size_t old_size = mFunctions.size();
      std::erase_if(mFunctions, [id](const Entry& e) { return e.id == id; });
      return mFunctions.size() != old_size;
    }

    // Remove all functions.
    void Clear() {
      mFunctions.clear();
    }

    // How many functions are stored.
    std::size_t Size() const {
      return mFunctions.size();
    }

    bool Empty() const {
      return mFunctions.empty();
    }

    // Call every stored function
    void CallAll(Args... args) const {
      for (const Entry& entry : mFunctions) {
        entry.fn(args...);
      }
    }

    //range access to all entries.
    auto Entries() {
      return std::views::all(mFunctions);
    }

    auto Entries() const {
      return std::views::all(mFunctions);
    }

    // View of the stored IDs
    auto IDs() const {
      return mFunctions | std::views::transform([](const Entry& e) { return e.id; });
    }

  private:
    std::vector<Entry> mFunctions;
    FunctionID mNextID = 0;
  };

}  // namespace cse498