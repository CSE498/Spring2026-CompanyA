/**
 * FunctionSet: A container that holds a collection of functions, all with the exact same signature.
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <exception>
#include <functional>
#include <string_view>
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
      const FunctionID id = mNextID++;
      mFunctions.push_back(Entry{id, std::move(fn)});
      return id;
    }

    // Remove a function by ID. Returns false if the ID is not found.
    bool RemoveFunction(FunctionID id) {
      const std::size_t old_size = mFunctions.size();
      mFunctions.erase(
          std::remove_if(mFunctions.begin(), mFunctions.end(),
                         [id](const Entry& e) { return e.id == id; }),
          mFunctions.end());
      return mFunctions.size() != old_size;
    }

    // Remove all functions and reset the ID.
    void Clear() {
      mFunctions.clear();
    }

    // How many functions are stored.
    std::size_t Size() const { return mFunctions.size(); }

    // Call every stored function using the same arguments, one at a time.
    // If one throws, catch it and continue calling the rest.
    void CallAll(
        Args... args,
        const std::function<void(FunctionID, std::string_view)>& on_error = nullptr) const {
      std::size_t failures = 0;

      for (const Entry& entry : mFunctions) {
        try {
          entry.fn(args...);
        } catch (const std::exception& e) {
          ++failures;
          if (on_error) {
            on_error(entry.id, e.what());
          }
        } catch (...) {
          ++failures;
          if (on_error) {
            on_error(entry.id, "Callback threw non-std exception");
          }
        }
      }

      mLastCallFailures = failures;
    }

    // Number of functions that threw during the most recent CallAll.
    std::size_t LastCallFailureCount() const { return mLastCallFailures; }

    // Number of functions that succeeded (did not throw) during the most recent CallAll.
    std::size_t LastCallSuccessCount() const { return Size() - mLastCallFailures; }

  private:
    struct Entry {
      FunctionID id;
      FunctionType fn;
    };

    std::vector<Entry> mFunctions;  // stored functions in insertion order
    FunctionID mNextID = 0;

    mutable std::size_t mLastCallFailures = 0;
  };

}  // namespace cse498