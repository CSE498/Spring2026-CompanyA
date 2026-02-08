/**
* @file MemoFunction.h
 * @author Henry Finfrock
 *
 * This class acts as a function wrapper, caching outputs to improve performance
 * Currently a first draft of the implementation
 */

#pragma once

#include <unordered_map>
#include <map>
#include <functional>
#include <iostream>

// Template for return and input types
template <typename R, typename... I>

class MemoFunction {
private:
    // Key type so multiple inputs can be used as a cache key
    using Key = std::tuple<I...>;

    // Cache to contain function outputs (should probably be unordered
    // for performance, doing map for ease of use for now)
    std::map<Key, R> mCache;

    // The function being wrapped
    std::function<R(I...)> mFunction;

    // Maybe want to include a limit to size of cache?
    //int mLimit;

public:
    // Constructor, instantiated as MemoFunction<R, I1, I2, ... , IN> name(function)
    explicit MemoFunction(std::function<R(I...)> mFunction) : mFunction(mFunction) {}

    // Provides () operator to easily use wrapped function w/ memoization
    R operator()(I... inputs)
    {
        Key key(inputs...);

        if (mCache.find(key) != mCache.end())
        {
            //std::cout << "from cache" << std::endl;
            return mCache[key];
        }

        R output = mFunction(inputs...);
        mCache[key] = output;
        //std::cout << "from function" << std::endl;
        return output;
    }

    // Getter for size
    [[nodiscard]] size_t cacheSize() const { return mCache.size(); }

    // Clears cache
    void cacheClear() { mCache.clear(); }
};