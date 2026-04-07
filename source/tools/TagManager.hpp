#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>  // For std::hash
#include <ranges>
#include <optional>
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
     *
     * Ownership: TagManager does not own the objects it tracks. It stores
     * plain integer IDs with no mechanism to detect external object destruction.
     * Callers are responsible for calling OnTagRemoved (or Clear) when an object
     * is destroyed; otherwise stale IDs will persist in query results.
     */
    class TagManager
    {
    public:
        using ObjectId = std::uint64_t;

        /// Errors that may be returned by FindSet().
        enum class FindSetError { EmptyTag, TagNotFound };

        /**
         * Default Constructor
         */
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
        using ObjectSet = std::unordered_set<ObjectId>;

        // Helper functions (mostly used in the Query function)
        static std::expected<void, FindSetError> AssertValidTag(std::string_view tag);
        std::expected<void, FindSetError> ValidateTags(const std::vector<std::string_view>& tags) const;
        std::expected<const ObjectSet*, FindSetError> FindSet(std::string_view tag) const;
        std::vector<std::reference_wrapper<const ObjectSet>> CollectExistingSets(const std::vector<std::string_view>& tags) const;
        std::optional<std::reference_wrapper<const ObjectSet>> FindSmallestIncludeSet(const std::vector<std::string_view>& includeTags) const;
        bool IsExcluded(ObjectId id, const std::vector<std::reference_wrapper<const ObjectSet>>& excludeSets) const;
        bool MatchesIncludeTags(ObjectId id, const std::vector<std::string_view>& includeTags) const;
        std::vector<ObjectId> QueryFromAllObjects(const std::vector<std::reference_wrapper<const ObjectSet>>& excludeSets) const;
        std::vector<ObjectId> QueryFromBaseSet(
            const ObjectSet& baseSet,
            const std::vector<std::string_view>& includeTags,
            const std::vector<std::reference_wrapper<const ObjectSet>>& excludeSets
            ) const;

        static bool IsValid(const std::expected<void, FindSetError>& result)
        {
            return result.has_value();
        }

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

            constexpr bool operator()(std::string_view a, std::string_view b) const noexcept { return a == b; }
            constexpr bool operator()(const std::string& a, const std::string& b) const noexcept { return a == b; }
            constexpr bool operator()(const std::string& a, std::string_view b) const noexcept { return std::string_view(a) == b; }
            constexpr bool operator()(std::string_view a, const std::string& b) const noexcept { return a == std::string_view(b); }
        };

        // TagMap now incorporates the Transparent Hash + Equality for no allocation lookup
        using TagMap = std::unordered_map<
            std::string,
            ObjectSet,
            TransparentStringHash,
            TransparentStringEqual
        >;


        bool Contains(ObjectId owner, std::string_view tag) const;
        // Reverse index (tag -> object)
        TagMap mTagToObjects;
        // 'Universe' of all tagged objects
        ObjectSet mAllObjects;
    };

}