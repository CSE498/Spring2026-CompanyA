/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A high-performance hash map using open addressing with robin hood hashing.
 * Robin Hood hashing resolves collisions by stealing from "rich" entries (close to their ideal slot)
 * and giving to "poor" ones (far from theirs), keeping probe distances short and lookup times even.
 * @note Status: PROPOSAL
 * @citation: Used Claude Code Opus 4.6 to help me understand the algorithm and implement it.
 **/

#pragma once

#include <cassert> // For assert
#include <cstddef> // For size_t
#include <functional> // For std::hash
#include <initializer_list> // For std::initializer_list
#include <iterator> // For std::forward_iterator_tag
#include <utility> // For std::swap
#include <vector> // For std::vector

namespace cse498 {

/// @class RobinHoodMap
/// @brief High-performance hash map using open addressing with robin hood hashing.
///
/// Provides significantly faster lookups and insertions than std::unordered_map
/// by keeping probe lengths short and memory access cache-friendly. Uses robin
/// hood hashing to minimize variance in probe distances during collisions, and
/// backward-shift deletion to avoid tombstone overhead.
///
/// Basic usage:
/// @code
///   cse498::RobinHoodMap<std::string, int> map;
///   map.Insert("health", 100);
///   map["mana"] = 50;
///   if (int * val = map.Find("health")) { /* use *val */ }
///   for (auto & [key, value] : map) { /* iterate */ }
/// @endcode
///
/// @warning Inserting elements (via Insert or operator[]) may trigger a rehash,
///          which invalidates ALL iterators and pointers. Do not hold iterators
///          or Find() pointers across operations that add new keys.
///
/// @tparam Key      Key type (must be hashable by Hash and comparable by KeyEqual)
/// @tparam Value    Mapped value type
/// @tparam Hash     Hash function object type (default: std::hash<Key>)
/// @tparam KeyEqual Key equality comparator type (default: std::equal_to<Key>)
template<typename Key, typename Value, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
class RobinHoodMap {
public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<Key, Value>;
    using size_type = std::size_t;
    using hasher = Hash;
    using key_equal = KeyEqual;

private:
    //  Internal bucket structure

    /// @brief A single slot in the open-addressing table.
    struct Bucket {
        value_type kv{}; ///< Stored key-value pair
        size_type probe_distance = 0; ///< Steps away from the ideal (hashed) position
        bool occupied = false; ///< True if this bucket holds a valid entry
    };

    //  Constants and member data

    static constexpr size_type DEFAULT_CAPACITY = 16;
    static constexpr double DEFAULT_MAX_LOAD_FACTOR = 0.60;


    std::vector<Bucket> buckets_;
    size_type size_ = 0;
    size_type capacity_;
    double max_load_factor_;
    Hash hash_fn_;
    KeyEqual key_eq_;

    //  Private helpers

    /// @brief Compute the ideal bucket index for a given key.
    [[nodiscard]] size_type BucketIndex(const Key& key) const { return hash_fn_(key) % capacity_; }

    /// @brief Would inserting one more element exceed the load factor? also asks ' is the table too full? '
    [[nodiscard]] bool ShouldRehash() const {
        return static_cast<double>(size_ + 1) / static_cast<double>(capacity_) > max_load_factor_;
    }

    /// @brief Grow the table to new_capacity buckets and reinsert all elements.
    void Rehash(size_type new_capacity) {
        assert(new_capacity > 0);
        std::vector<Bucket> old_buckets = std::move(buckets_);
        capacity_ = new_capacity;
        buckets_.assign(capacity_, Bucket{});
        size_ = 0;

        for (auto& bucket: old_buckets) {
            if (bucket.occupied) {
                InsertEntry(std::move(bucket.kv));
            }
        }
    }

    /// @brief Locate the bucket holding key, or return capacity_ if absent.
    ///
    /// Takes advantage of robin hood early-termination: if the current bucket's
    /// probe_distance is less than what ours would be at this point, the key
    /// cannot exist any further along the chain.
    [[nodiscard]] size_type FindIndex(const Key& key) const {
        size_type idx = BucketIndex(key);
        size_type dist = 0;

        while (true) {
            if (!buckets_[idx].occupied)
                return capacity_;
            if (buckets_[idx].probe_distance < dist)
                return capacity_;
            if (key_eq_(buckets_[idx].kv.first, key))
                return idx;
            ++dist;
            idx = (idx + 1) % capacity_;
        }
    }

    /// @brief Insert an entry whose key is known to not already exist.
    ///        The caller must have already checked for duplicates and rehashed
    ///        if necessary.
    /// @return Bucket index where the original entry was placed.
    size_type InsertEntry(value_type entry) {
        size_type idx = BucketIndex(entry.first);
        size_type dist = 0;
        size_type original_idx = capacity_; // sentinel: not yet placed by swap

        while (true) {
            // Found an empty slot — place whatever we're carrying.
            if (!buckets_[idx].occupied) {
                buckets_[idx].kv = std::move(entry);
                buckets_[idx].probe_distance = dist;
                buckets_[idx].occupied = true;
                ++size_;
                return (original_idx != capacity_) ? original_idx : idx;
            }

            // Robin hood: displace the "richer" element (shorter probe distance).
            if (buckets_[idx].probe_distance < dist) {
                std::swap(entry, buckets_[idx].kv);
                std::swap(dist, buckets_[idx].probe_distance);
                if (original_idx == capacity_) {
                    original_idx = idx; // Our original entry now lives here.
                }
            }

            ++dist;
            idx = (idx + 1) % capacity_;
        }
    }

    /// @brief Backward-shift deletion at bucket idx.
    ///
    /// After removing the entry, shifts subsequent displaced elements one
    /// position backward to fill the gap.  This avoids tombstones entirely
    /// and keeps probe-distance invariants intact.
    void EraseAt(size_type idx) {
        buckets_[idx].occupied = false;
        --size_;

        size_type next = (idx + 1) % capacity_;
        while (buckets_[next].occupied && buckets_[next].probe_distance > 0) {
            buckets_[idx] = std::move(buckets_[next]);
            --buckets_[idx].probe_distance;
            buckets_[next].occupied = false;
            idx = next;
            next = (next + 1) % capacity_;
        }
    }

public:
    //  Iterator

    /// @brief Forward iterator over stored key-value pairs.
    /// @warning Invalidated by any insert, operator[], or rehash operation.
    class Iterator {
        friend class RobinHoodMap; // Allow map to construct iterators

    private:
        Bucket* ptr_ = nullptr;
        size_type index_ = 0;
        size_type cap_ = 0;

        /// Skip past unoccupied buckets.
        void SkipEmpty() {
            while (index_ < cap_ && !ptr_[index_].occupied) {
                ++index_;
            }
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<Key, Value>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator() = default;
        Iterator(Bucket* ptr, size_type index, size_type cap) : ptr_(ptr), index_(index), cap_(cap) { SkipEmpty(); }

        Iterator(const Iterator&) = default;
        Iterator& operator=(const Iterator&) = default;

        reference operator*() const { return ptr_[index_].kv; }
        pointer operator->() const { return &ptr_[index_].kv; }

        Iterator& operator++() {
            ++index_;
            SkipEmpty();
            return *this;
        }
        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator& o) const { return index_ == o.index_; }
        bool operator!=(const Iterator& o) const { return index_ != o.index_; }
    };

    /// @brief Const forward iterator over stored key-value pairs.
    class ConstIterator {
        friend class RobinHoodMap;

    private:
        const Bucket* ptr_ = nullptr;
        size_type index_ = 0;
        size_type cap_ = 0;

        void SkipEmpty() {
            while (index_ < cap_ && !ptr_[index_].occupied) {
                ++index_;
            }
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<Key, Value>;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;

        ConstIterator() = default;
        ConstIterator(const Bucket* ptr, size_type index, size_type cap) : ptr_(ptr), index_(index), cap_(cap) {
            SkipEmpty();
        }

        ConstIterator(const ConstIterator&) = default;
        ConstIterator& operator=(const ConstIterator&) = default;

        reference operator*() const { return ptr_[index_].kv; }
        pointer operator->() const { return &ptr_[index_].kv; }

        ConstIterator& operator++() {
            ++index_;
            SkipEmpty();
            return *this;
        }
        ConstIterator operator++(int) {
            ConstIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const ConstIterator& o) const { return index_ == o.index_; }
        bool operator!=(const ConstIterator& o) const { return index_ != o.index_; }
    };

    using iterator = Iterator;
    using const_iterator = ConstIterator;

    //  Construction

    /// @brief Construct an empty RobinHoodMap.
    /// @param initial_capacity Starting bucket count (default 16).
    /// @param max_load_factor  Ratio that triggers rehash (default 0.85).
    /// @param hash_fn          Hash function instance.
    /// @param key_eq           Key equality comparator instance.
    explicit RobinHoodMap(size_type initial_capacity = DEFAULT_CAPACITY,
                          double max_load_factor = DEFAULT_MAX_LOAD_FACTOR, const Hash& hash_fn = Hash{},
                          const KeyEqual& key_eq = KeyEqual{}) :
        buckets_(initial_capacity > 0 ? initial_capacity : 1, Bucket{}), size_(0),
        capacity_(initial_capacity > 0 ? initial_capacity : 1), max_load_factor_(max_load_factor), hash_fn_(hash_fn),
        key_eq_(key_eq) {
        assert(max_load_factor_ > 0.0 && max_load_factor_ <= 1.0);
    }

    /// @brief Construct from an initializer list of key-value pairs.
    RobinHoodMap(std::initializer_list<value_type> init, const Hash& hash_fn = Hash{},
                 const KeyEqual& key_eq = KeyEqual{}) :
        RobinHoodMap(DEFAULT_CAPACITY, DEFAULT_MAX_LOAD_FACTOR, hash_fn, key_eq) {
        Reserve(init.size());
        for (const auto& pair: init) {
            Insert(pair.first, pair.second);
        }
    }

    RobinHoodMap(const RobinHoodMap&) = default;
    RobinHoodMap& operator=(const RobinHoodMap&) = default;
    RobinHoodMap(RobinHoodMap&&) noexcept = default;
    RobinHoodMap& operator=(RobinHoodMap&&) noexcept = default;
    ~RobinHoodMap() = default;

    //  Capacity

    /// @brief Return the number of stored key-value pairs.
    [[nodiscard]] size_type Size() const { return size_; }

    /// @brief Check whether the map contains no elements.
    [[nodiscard]] bool Empty() const { return size_ == 0; }

    /// @brief Return the current number of buckets.
    [[nodiscard]] size_type BucketCount() const { return capacity_; }

    /// @brief Return the current load factor (size / bucket_count).
    [[nodiscard]] double GetLoadFactor() const { return static_cast<double>(size_) / static_cast<double>(capacity_); }

    /// @brief Return the maximum load factor threshold.
    [[nodiscard]] double GetMaxLoadFactor() const { return max_load_factor_; }

    /// @brief Set the maximum load factor threshold.
    /// @param mlf Must be in (0.0, 1.0].
    void SetMaxLoadFactor(double mlf) {
        assert(mlf > 0.0 && mlf <= 1.0);
        max_load_factor_ = mlf;
    }

    /// @brief Pre-allocate enough buckets for at least count elements
    ///        without exceeding the max load factor.
    void Reserve(size_type count) {
        size_type needed = static_cast<size_type>(static_cast<double>(count) / max_load_factor_) +
                           1; // +1 to round up, ensuring we never undershoot the required capacity
        if (needed > capacity_) {
            Rehash(needed);
        }
    }

    //  Lookup

    /// @brief Find the value associated with a key.
    /// @return Pointer to the value if found, nullptr otherwise.
    [[nodiscard]] Value* Find(const Key& key) {
        size_type idx = FindIndex(key);
        if (idx == capacity_)
            return nullptr;
        return &buckets_[idx].kv.second;
    }

    /// @brief Find the value associated with a key (const).
    /// @return Pointer to the value if found, nullptr otherwise.
    [[nodiscard]] const Value* Find(const Key& key) const {
        size_type idx = FindIndex(key);
        if (idx == capacity_)
            return nullptr;
        return &buckets_[idx].kv.second;
    }

    /// @brief Check whether the map contains a given key.
    /// Unlike Find(), which returns a pointer to the value, this only
    /// reports presence and avoids exposing internal storage.
    [[nodiscard]] bool Contains(const Key& key) const { return FindIndex(key) != capacity_; }

    //  Modifiers

    /// @brief Insert a key-value pair, or update the value if the key exists.
    /// @return true if a new element was inserted; false if an existing value
    ///         was updated.
    /// @warning May trigger a rehash, invalidating all iterators and pointers.
    bool Insert(const Key& key, const Value& value) {
        size_type existing = FindIndex(key);
        if (existing != capacity_) {
            buckets_[existing].kv.second = value;
            return false;
        }

        if (ShouldRehash()) {
            Rehash(capacity_ * 2);
        }
        InsertEntry(value_type{key, value});
        return true;
    }

    /// @brief Insert with move semantics for the value.
    bool Insert(const Key& key, Value&& value) {
        size_type existing = FindIndex(key);
        if (existing != capacity_) {
            buckets_[existing].kv.second = std::move(value);
            return false;
        }

        if (ShouldRehash()) {
            Rehash(capacity_ * 2);
        }
        InsertEntry(value_type{key, std::move(value)});
        return true;
    }

    /// @brief Access value by key.  Default-constructs a new value if the key
    ///        is not already present.
    /// @warning May trigger a rehash, invalidating all iterators and pointers.
    Value& operator[](const Key& key) {
        size_type existing = FindIndex(key);
        if (existing != capacity_) {
            return buckets_[existing].kv.second;
        }

        if (ShouldRehash()) {
            Rehash(capacity_ * 2);
        }
        size_type idx = InsertEntry(value_type{key, Value{}});
        return buckets_[idx].kv.second;
    }

    /// @brief Remove the element with the given key.
    /// @return true if an element was removed; false if the key was not found.
    bool Erase(const Key& key) {
        size_type idx = FindIndex(key);
        if (idx == capacity_)
            return false;
        EraseAt(idx);
        return true;
    }

    /// @brief Remove all elements.  Bucket count stays the same.
    void Clear() {
        buckets_.assign(capacity_, Bucket{});
        size_ = 0;
    }

    //  Iteration  (lowercase names required for range-based for loops)

    [[nodiscard]] iterator begin() { return Iterator(buckets_.data(), 0, capacity_); }

    [[nodiscard]] iterator end() { return Iterator(buckets_.data(), capacity_, capacity_); }

    [[nodiscard]] const_iterator begin() const { return ConstIterator(buckets_.data(), 0, capacity_); }

    [[nodiscard]] const_iterator end() const { return ConstIterator(buckets_.data(), capacity_, capacity_); }

    [[nodiscard]] const_iterator cbegin() const { return ConstIterator(buckets_.data(), 0, capacity_); }

    [[nodiscard]] const_iterator cend() const { return ConstIterator(buckets_.data(), capacity_, capacity_); }
};

} // namespace cse498
