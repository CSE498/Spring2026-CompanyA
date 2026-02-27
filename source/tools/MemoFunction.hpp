/**
 * @file MemoFunction.hpp
 * @author Henry Finfrock
 *
 * This class acts as a function wrapper, caching outputs to improve performance
 * Initial post-review implementation, subject to change
 */

#pragma once

#include <optional>
#include <unordered_map>
#include <map>
#include <functional>
#include <deque>

namespace cse498 {

 // Template for return and input types
 template <typename R, typename... I>

 class MemoFunction {
 private:
  // Key type so multiple inputs can be used as a cache key
  using Key = std::tuple<I...>;

  std::function<R(I...)> mWrappedFunction;

  // Cache to contain function outputs (ordered map for now,
  // look into using RobinHoodMap)
  std::map<Key, R> mCache;

  // Queue that records order of caching, so when the cache reaches its
  // limit it will remove the oldest
  std::deque<Key> mQueue;

  // Size limit for cache (0 being no limit)
  std::optional<size_t> mLimit = std::nullopt;

  // Removes oldest items until size limit is no longer exceeded
  void EvictCache()
  {
   while (mLimit && CacheSize() > *mLimit)
   {
    Key oldest = mQueue.front();
    mQueue.pop_front();
    mCache.erase(oldest);
   }
  }

 public:
  // Constructor, instantiated as MemoFunction<R, I1, I2, ... , IN> name(function)
  explicit MemoFunction(std::function<R(I...)> mWrappedFunction) : mWrappedFunction(mWrappedFunction) {}

  // Main memoization function with cache eviction when size limit is reached
  R operator()(I... inputs)
  {
   Key key(inputs...);

   if (mCache.find(key) != mCache.end()){ return mCache[key]; }

   R output = mWrappedFunction(inputs...);
   mCache[key] = output;
   mQueue.push_back(key);

   EvictCache();

   return output;
  }

  [[nodiscard]] size_t CacheSize() const { return mCache.size(); }

  void CacheClear() { mCache.clear(); mQueue.clear(); }

  void SetLimit(size_t limit) { mLimit = limit; EvictCache(); }

  void RemoveLimit() { mLimit = std::nullopt; }

  bool IsCached(Key key) const { return mCache.find(key) != mCache.end(); }
 };
}