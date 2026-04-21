/**
 * @file MemoFunction.hpp
 * @author Henry Finfrock
 *
 * This class acts as a function wrapper, caching outputs to improve performance
 * Initial post-review implementation, subject to change
 */

#pragma once

#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <unordered_map>

namespace cse498 {

// Template for return and input types
template<typename R, typename... I>

class MemoFunction {
private:
    // Key type so multiple inputs can be used as a cache key
    using Key = std::tuple<I...>;
    std::function<R(I...)> mWrappedFunction;
    std::map<Key, R> mCache;
    std::deque<Key> mQueue;
    std::optional<size_t> mLimit = std::nullopt;

    /**
     * Removes oldest items until size limit is no longer exceeded
     */
    void EvictCache() noexcept {
        while (mLimit && CacheSize() > *mLimit) {
            Key oldest = mQueue.front();
            mQueue.pop_front();
            mCache.erase(oldest);
        }
    }

public:
    /**
     * Constructor, instantiated as MemoFunction<R, I1, I2, ... , IN> name(function)
     * @param mWrappedFunction The function to be wrapped
     */
    explicit MemoFunction(std::function<R(I...)> mWrappedFunction) noexcept : mWrappedFunction(mWrappedFunction) {}

    /**
     * Main memoization function with cache eviction when size limit is reached
     * @param inputs The inputs corresponding to the wrapped function
     * @return The output of the wrapped function
     */
    R operator()(I... inputs) {
        Key key(inputs...);

        if (mCache.contains(key)) {
            return mCache[key];
        }

        R output = mWrappedFunction(inputs...);
        mCache[key] = output;
        mQueue.push_back(key);

        EvictCache();

        return output;
    }

    /**
     * @return The number of items currently in the cache
     */
    [[nodiscard]] size_t CacheSize() const noexcept { return mCache.size(); }

    /**
     * Clears the cache and the queue
     */
    void CacheClear() noexcept {
        mCache.clear();
        mQueue.clear();
    }

    /**
     * Sets maximum number of items in cache
     * @param Limit to be set
     */
    void SetLimit(size_t limit) noexcept {
        mLimit = limit;
        EvictCache();
    }

    /**
     * Removes limit from cache
     */
    void RemoveLimit() noexcept { mLimit = std::nullopt; }

    /**
     * Checks if key is in cache
     * @param Key to check
     * @return True if key exists in cache
     */
    bool IsCached(const Key& key) const noexcept { return mCache.contains(key); }
};
} // namespace cse498
