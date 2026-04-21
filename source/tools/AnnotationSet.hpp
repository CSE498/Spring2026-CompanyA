/**
 * @file AnnotationSet.hpp
 * @brief Set of string tags owned by a game object, optionally coordinated with a TagManager.
 */
#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace cse498 {

class TagManager;

/**
 * @brief Tags (non-empty strings) attached to an ObjectId, stored in an unordered set.
 *
 * When a TagManager pointer is supplied, add/remove paths can notify the manager (hooks are
 * reserved for future use). Tags must not be empty; callers violating this hit an assertion.
 */
class AnnotationSet {
private:
    static void AssertValidTag(std::string_view tag);

    void NotifyAdded(std::string_view tag) noexcept;
    void NotifyRemoved(std::string_view tag) noexcept;

    TagManager* mManager{nullptr};
    std::unordered_set<std::string> mTags;

public:
    using ObjectId = std::uint64_t;

    /**
     * @brief Construct an empty tag set for @p owner.
     * @param owner Stable object id this set describes.
     * @param manager Optional tag manager to attach (may be nullptr).
     */
    explicit AnnotationSet(ObjectId owner, TagManager* manager = nullptr) : mManager(manager), mOwner(owner) {}

    /// @brief Associate this set with @p manager (does not migrate existing tags).
    void AttachManager(TagManager* manager) noexcept { mManager = manager; }

    /// @brief Object id this annotation set belongs to.
    ObjectId Owner() const noexcept { return mOwner; }
    /// @brief Number of distinct tags.
    std::size_t Size() const noexcept { return mTags.size(); }
    /// @brief True if there are no tags.
    bool Empty() const noexcept { return mTags.empty(); }

    /**
     * @brief Insert a tag.
     * @param tag Non-empty tag string.
     * @return false if the tag was already present, true if inserted.
     */
    bool AddTag(std::string_view tag);

    /**
     * @brief Remove a tag.
     * @param tag Tag to remove.
     * @return true if a tag was removed, false if it was absent.
     */
    bool RemoveTag(std::string_view tag);

    /// @brief True if @p tag is in the set.
    bool HasTag(std::string_view tag) const;

    /// @brief Remove every tag (notifies manager per tag when attached).
    void Clear();

    /**
     * @brief Copy of all tags as a vector (stable for serialization or ordered iteration).
     */
    std::vector<std::string> ToVector() const {
        std::vector<std::string> out;
        out.reserve(mTags.size());
        for (const auto& t: mTags)
            out.push_back(t);
        return out;
    }

    /**
     * @brief Invoke @p func on each tag.
     * @tparam Func Callable `void(const std::string&)`.
     * @param func Visitor.
     */
    template<typename Func>
    void ForEach(Func&& func) const {
        for (const auto& tag: mTags) {
            func(tag);
        }
    }

    /**
     * @brief Collect tags satisfying @p pred.
     * @tparam Pred Callable `bool(const std::string&)`.
     * @param pred Filter predicate.
     */
    template<typename Pred>
    [[nodiscard]] std::vector<std::string> Filter(Pred&& pred) const {
        std::vector<std::string> result;
        std::copy_if(mTags.begin(), mTags.end(), std::back_inserter(result), pred);
        return result;
    }

    /**
     * @brief True if @p pred holds for at least one tag.
     * @tparam Pred Callable `bool(const std::string&)`.
     */
    template<typename Pred>
    [[nodiscard]] bool AnyOf(Pred&& pred) const {
        for (const auto& tag: mTags) {
            if (pred(tag))
                return true;
        }
        return false;
    }

    /**
     * @brief True if @p pred holds for every tag (vacuously true when empty).
     * @tparam Pred Callable `bool(const std::string&)`.
     */
    template<typename Pred>
    [[nodiscard]] bool AllOf(Pred&& pred) const {
        for (const auto& tag: mTags) {
            if (!pred(tag))
                return false;
        }
        return true;
    }

    /**
     * @brief True if no tag satisfies @p pred.
     * @tparam Pred Callable `bool(const std::string&)`.
     */
    template<typename Pred>
    [[nodiscard]] bool NoneOf(Pred&& pred) const {
        return !AnyOf(std::forward<Pred>(pred));
    }

    /**
     * @brief Erase every tag matching @p pred.
     * @tparam Pred Callable `bool(const std::string&)`.
     * @return Count of removed tags.
     */
    template<typename Pred>
    std::size_t RemoveIf(Pred&& pred) {
        std::size_t removed = 0;
        for (auto it = mTags.begin(); it != mTags.end();) {
            if (pred(*it)) {
                NotifyRemoved(*it);
                it = mTags.erase(it);
                ++removed;
            } else {
                ++it;
            }
        }
        return removed;
    }

    using const_iterator = std::unordered_set<std::string>::const_iterator;
    /// @brief Iterator to first tag.
    const_iterator begin() const noexcept { return mTags.begin(); }
    /// @brief Past-the-end iterator.
    const_iterator end() const noexcept { return mTags.end(); }

private:
    ObjectId mOwner{};
};
} // namespace cse498
