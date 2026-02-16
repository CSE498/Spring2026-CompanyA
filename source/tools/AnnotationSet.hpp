// AnnotationSet.h
#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace cse498 {

    class TagManager;

    class AnnotationSet {
    private:
        static void AssertValidTag(std::string_view tag) {
            
            assert(!tag.empty() && "AnnotationSet: tag must not be empty");

        }

        void NotifyAdded(std::string_view tag) noexcept;
        void NotifyRemoved(std::string_view tag) noexcept;

        
        TagManager* mManager{nullptr};
        std::unordered_set<std::string> mTags;

    public:
        using ObjectId = std::uint64_t;

        explicit AnnotationSet(ObjectId owner, TagManager* manager = nullptr)
            : mOwner(owner), mManager(manager) {}

        void AttachManager(TagManager* manager) noexcept { mManager = manager; }

        ObjectId Owner() const noexcept { return mOwner; }
        std::size_t Size() const noexcept { return mTags.size(); }
        bool Empty() const noexcept { return mTags.empty(); }

        // Add a tag. Returns false is not added, otherwise returns true
        bool AddTag(std::string_view tag) {
            AssertValidTag(tag);

            
            std::string owned(tag);

            auto [it, inserted] = mTags.insert(std::move(owned));
            if (!inserted) {
                return false; 
            }

            NotifyAdded(*it);
            return true;
        }

        // Remove a tag. Returns true if removed, false if it did not exist
        bool RemoveTag(std::string_view tag) {
            AssertValidTag(tag);

            auto it = mTags.find(std::string(tag));
            if (it == mTags.end()) {
                return false; 
            }

            std::string removed = *it;
            mTags.erase(it);

            NotifyRemoved(removed);
            return true;
        }

        // Check if a tag exists
        bool HasTag(std::string_view tag) const {
            AssertValidTag(tag);
            return mTags.find(std::string(tag)) != mTags.end();
        }

        // Removes all tags
        void Clear() {
            if (!mManager) {
                mTags.clear();
                return;
            }

            for (const auto& t : mTags) {
                NotifyRemoved(t);
            }
            mTags.clear();
        }

        std::vector<std::string> ToVector() const {
            std::vector<std::string> out;
            out.reserve(mTags.size());
            for (const auto& t : mTags) out.push_back(t);
            return out;
        }

        using const_iterator = std::unordered_set<std::string>::const_iterator;
        const_iterator begin() const noexcept { return mTags.begin(); }
        const_iterator end() const noexcept { return mTags.end(); }

    private:
        ObjectId mOwner{};

    };
};