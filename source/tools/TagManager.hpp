#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>  // For std::hash
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cse498
{

    /**
     * Manages associations between object IDs and string tags.
     *
     * Internally maintains a reverse index from tag -> set of object IDs,
     * and a "universe" set containing all objects that have ever been tagged.
     */
    class TagManager
    {
    public:
        using ObjectId = std::uint64_t;

        /// Errors that may be returned by FindSet().
        enum class FindSetError { EmptyTag, TagNotFound };

        TagManager() = default;

        void OnTagAdded(ObjectId owner, std::string_view tag);
        void OnTagRemoved(ObjectId owner, std::string_view tag);
        
        std::vector<ObjectId> Query(
            const std::vector<std::string_view>& includeTags,
            const std::vector<std::string_view>& excludeTags = {}
        ) const;

        bool HasTag(ObjectId owner, std::string_view tag) const;
        void Clear();

    private:
        static void AssertValidTag(std::string_view tag);

        using ObjectSet = std::unordered_set<ObjectId>;

        // Used ChatGPT to help with string_view issue without it having to create a  temporary string to find tags (redundant)
        // This enables transparent hash and equality (below), which means no allocation on lookup
        struct TransparentStringHash
        {
            using is_transparent = void;

            size_t operator()(std::string_view sv) const noexcept
            {
                return std::hash<std::string_view>{}(sv);
            }

            size_t operator()(const std::string& s) const noexcept
            {
                return std::hash<std::string_view>{}(s);
            }
        };

        // Transparent Equality
        struct TransparentStringEqual
        {
            using is_transparent = void;

            bool operator()(std::string_view a, std::string_view b) const noexcept { return a == b; }
            bool operator()(const std::string& a, const std::string& b) const noexcept { return a == b; }
            bool operator()(const std::string& a, std::string_view b) const noexcept { return std::string_view(a) == b; }
            bool operator()(std::string_view a, const std::string& b) const noexcept { return a == std::string_view(b); }
        };

        // TagMap now incorporates the Transparent Hash + Equality for no allocation lookup
        using TagMap = std::unordered_map<
            std::string,
            ObjectSet,
            TransparentStringHash,
            TransparentStringEqual
        >;

        std::expected<const ObjectSet*, FindSetError> FindSet(std::string_view tag) const;
        bool Contains(ObjectId owner, std::string_view tag) const;
        // Reverse index (tag -> object)
        TagMap mTagToObjects;
        // 'Universe' of all tagged objects
        ObjectSet mAllObjects;
    };

}