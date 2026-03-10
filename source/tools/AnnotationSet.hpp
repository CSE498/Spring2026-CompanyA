// AnnotationSet.hpp
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
        static void AssertValidTag(std::string_view tag);

        void NotifyAdded(std::string_view tag) noexcept;
        void NotifyRemoved(std::string_view tag) noexcept;

        
        TagManager* mManager{nullptr};
        std::unordered_set<std::string> mTags;

    public:
        using ObjectId = std::uint64_t;

        explicit AnnotationSet(ObjectId owner, TagManager* manager = nullptr) : mManager(manager), mOwner(owner) {}

        void AttachManager(TagManager* manager) noexcept { mManager = manager; }

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