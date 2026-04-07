// AnnotationSet.hpp
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

    class AnnotationSet {
    private:
        static void AssertValidTag(std::string_view tag);

        void NotifyAdded(std::string_view tag) noexcept;
        void NotifyRemoved(std::string_view tag) noexcept;


        TagManager *mManager{nullptr};
        std::unordered_set<std::string> mTags;

    public:
        using ObjectId = std::uint64_t;

        explicit AnnotationSet(ObjectId owner, TagManager *manager = nullptr) : mManager(manager), mOwner(owner) {}

        void AttachManager(TagManager *manager) noexcept { mManager = manager; }

        ObjectId Owner() const noexcept { return mOwner; }
        std::size_t Size() const noexcept { return mTags.size(); }
        bool Empty() const noexcept { return mTags.empty(); }

        // Add a tag. Returns false is not added, otherwise returns true
        bool AddTag(std::string_view tag);

        // Remove a tag. Returns true if removed, false if it did not exist
        bool RemoveTag(std::string_view tag);

        // Check if a tag exists
        bool HasTag(std::string_view tag) const;

        // Removes all tags
        void Clear();

        /// Returns a copy of all tags as a vector, useful for serialization or iteration
        /// where set semantics are not needed.
        std::vector<std::string> ToVector() const {
            std::vector<std::string> out;
            out.reserve(mTags.size());
            for (const auto &t: mTags)
                out.push_back(t);
            return out;
        }


        /// Apply a function to each tag.
        /// @tparam Func  Callable taking const std::string&.
        /// @param func   The function to apply.
        template<typename Func>
        void ForEach(Func &&func) const {
            for (const auto &tag: mTags) {
                func(tag);
            }
        }

        /// Get all tags matching a predicate.
        /// @tparam Pred  Callable taking const std::string& and returning bool.
        /// @param pred   The predicate to test.
        /// @return Vector of matching tags.
        template<typename Pred>
        [[nodiscard]] std::vector<std::string> Filter(Pred &&pred) const {
            std::vector<std::string> result;
            std::copy_if(mTags.begin(), mTags.end(), std::back_inserter(result), pred);
            return result;
        }

        /// Check if any tag matches a predicate.
        /// @tparam Pred  Callable taking const std::string& and returning bool.
        /// @param pred   The predicate to test.
        /// @return True if at least one tag matches.
        template<typename Pred>
        [[nodiscard]] bool AnyOf(Pred &&pred) const {
            for (const auto &tag: mTags) {
                if (pred(tag))
                    return true;
            }
            return false;
        }

        /// Check if all tags match a predicate.
        /// @tparam Pred  Callable taking const std::string& and returning bool.
        /// @param pred   The predicate to test.
        /// @return True if all tags match (or set is empty).
        template<typename Pred>
        [[nodiscard]] bool AllOf(Pred &&pred) const {
            for (const auto &tag: mTags) {
                if (!pred(tag))
                    return false;
            }
            return true;
        }

        /// Check if no tags match a predicate.
        /// @tparam Pred  Callable taking const std::string& and returning bool.
        /// @param pred   The predicate to test.
        /// @return True if no tags match.
        template<typename Pred>
        [[nodiscard]] bool NoneOf(Pred &&pred) const {
            return !AnyOf(std::forward<Pred>(pred));
        }

        /// Remove all tags matching a predicate.
        /// @tparam Pred  Callable taking const std::string& and returning bool.
        /// @param pred   The predicate to test.
        /// @return Number of tags removed.
        template<typename Pred>
        std::size_t RemoveIf(Pred &&pred) {
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
        const_iterator begin() const noexcept { return mTags.begin(); }
        const_iterator end() const noexcept { return mTags.end(); }

    private:
        ObjectId mOwner{};
    };
}; // namespace cse498
