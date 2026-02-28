/**
 * @file MemoFunction.hpp
 * @author Henry Finfrock
 *
 * This class acts as a function wrapper, caching outputs to improve performance
 * Initial implementation, subject to change
 */

#pragma once

#include <unordered_map>
#include <map>
#include <functional>
#include <deque>
//#include <iostream>

namespace cse498 {

 // Template for return and input types
 template <typename R, typename... I>

 class MemoFunction {
 private:
  // Key type so multiple inputs can be used as a cache key
  using Key = std::tuple<I...>;

  // Cache to contain function outputs (should probably be unordered
  // for performance, doing map for ease of use for now)
  std::map<Key, R> mCache;

  // Queue that records order of caching, so when the cache reaches its
  // limit it will remove the oldest
  std::deque<Key> mQueue;

  // The function being wrapped
  std::function<R(I...)> mFunction;

  // Size limit for cache (0 and less being no limit)
  size_t mLimit = 0;

 public:
  // Constructor, instantiated as MemoFunction<R, I1, I2, ... , IN> name(function)
  explicit MemoFunction(std::function<R(I...)> mFunction) : mFunction(mFunction) {}

  // Main memoization function with cache deletion when size limit is reached
  R operator()(I... inputs)
  {
   Key key(inputs...);

   if (mCache.contains(key))
   {
    //std::cout << "from cache" << std::endl;
    return mCache[key];
   }

   R output = mFunction(inputs...);
   mCache[key] = output;
   mQueue.push_back(key);
   //std::cout << "from function" << std::endl;

   if (mLimit > 0 && CacheSize() > mLimit) {
    Key oldest = mQueue.front();
    mQueue.pop_front();
    mCache.erase(oldest);
   }

   return output;
  }

  [[nodiscard]] size_t CacheSize() const { return mCache.size(); }

  void CacheClear() { mCache.clear(); mQueue.clear(); }

  void SetLimit(const int limit) { mLimit = limit; }

  bool IsCached(Key key) const { return mCache.contains(key); }
 };
}