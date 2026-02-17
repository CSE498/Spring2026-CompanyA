#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cse498 {

    class TagManager {
    public:
        using ObjectId = std::uint64_t;

        TagManager() = default;

        // Called by AnnotationSet when an object's tag changes.
        void OnTagAdded(ObjectId owner, std::string_view tag);
        void OnTagRemoved(ObjectId owner, std::string_view tag);

        // Query: must have all tags in includeTags, must have none in excludeTags.
        // If includeTags is empty -> start from "all known objects" then exclude.
        std::vector<ObjectId> Query(
            const std::vector<std::string_view>& includeTags,
            const std::vector<std::string_view>& excludeTags = {}
        ) const;

        // Tag management helper functions
        bool HasTag(ObjectId owner, std::string_view tag);
        void Clear();

    private:
        static void AssertValidTag(std::string_view tag);

        using ObjectSet = std::unordered_set<ObjectId>;
        using TagMap = std::unordered_map<std::string, ObjectSet>;

        const ObjectSet* FindSet(std::string_view tag);

        TagMap    mTagToObjects;
        ObjectSet mAllObjects; // "universe" of all known owners that ever had a tag
    };

}