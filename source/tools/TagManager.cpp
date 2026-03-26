#include "TagManager.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

namespace cse498
{
    /**
    * Validates that a tag is non-empty.
    *
    * In debug builds, triggers an assertion if the tag is empty.
    *
    * @param tag The tag string to validate.
    */
    std::expected<void, TagManager::FindSetError>
    TagManager::AssertValidTag(std::string_view tag)
    {
        if (tag.empty())
        {
            return std::unexpected(FindSetError::EmptyTag);
        }

        return {};
    }

    /**
     * Finds the objects associated with a/multiple tags.
     *
     * Performs a lookup in the internal tag-to-object map.
     *
     * @param tag The tag to search for.
     * @return Reference to the ObjectSet associated with the tag.
     */
    std::expected<const TagManager::ObjectSet*, TagManager::FindSetError>
    TagManager::FindSet(std::string_view tag) const
    {
        if (tag.empty())
        {
            return std::unexpected(FindSetError::EmptyTag);
        }

        auto it = mTagToObjects.find(tag);
        if (it == mTagToObjects.end())
        {
            return std::unexpected(FindSetError::TagNotFound);
        }

        return &it->second;
    }

    /**
     * Checks whether an object is present in the ObjectSet for a tag. Suggested as a helper function
     * @param owner The object identifier to test.
     * @param tag The tag name.
     * @return true if owner is associated with tag; false otherwise.
     */
    bool TagManager::Contains(ObjectId owner, std::string_view tag) const
    {
        auto setResult = FindSet(tag);
        if (!setResult)
            return false;

        const ObjectSet* setPtr = setResult.value();;
        return setPtr->find(owner) != setPtr->end();
    }

    /**
    * Adds an object to a tag.
    *
    * Inserts the object into the tag's object set. If the tag does not already exist, it is created. The object is also added to the universe of known objects.
    *
    * @param owner The object identifier.
    * @param tag The tag to associate with the object.
    *
    * @throws std::invalid_argument if the tag is empty.
    */
    void TagManager::OnTagAdded(ObjectId owner, std::string_view tag)
    {
        AssertValidTag(tag);

        mAllObjects.insert(owner);

        // Insertion requires owning key storage => std::string construction is expected here.
        auto& setForTag = mTagToObjects[std::string(tag)];
        setForTag.insert(owner);
    }

    /**
     * Removes a tag from an object.
     *
     * If the tag exists, the object is removed from its associated set. If the tag's set becomes empty, the tag entry is removed entirely. The object remains in the global object set.
     *
     * @param owner The object identifier.
     * @param tag The tag to remove.
     *
     * @throws std::invalid_argument if the tag is empty.
     */
    void TagManager::OnTagRemoved(ObjectId owner, std::string_view tag)
    {
        AssertValidTag(tag);

        auto it = mTagToObjects.find(tag);
        if (it == mTagToObjects.end())
        {
            return;
        }

        it->second.erase(owner);

        // Remove empty tag buckets
        if (it->second.empty())
        {
            mTagToObjects.erase(it);
        }

        // Keep owner in mAllObjects (universe) even if it has no tags now.
    }

    /**
     * Queries objects by inclusion and exclusion tags.
     *
     * Returns all objects that:
     * - Contain all tags in includeTags
     * - Contain none of the tags in excludeTags
     *
     * If includeTags is empty, all known objects are considered as the base set.
     *
     * @param includeTags Tags that objects must contain.
     * @param excludeTags Tags that objects must not contain.
     * @return A vector of object IDs matching the query conditions.
     *
     * @throws std::invalid_argument if any tag is empty.
     */
    std::vector<TagManager::ObjectId>
    TagManager::Query(
        const std::vector<std::string_view>& includeTags,
        const std::vector<std::string_view>& excludeTags) const
    {
        ValidateTags(includeTags);
        ValidateTags(excludeTags);

        const auto excludeSets = CollectExistingSets(excludeTags);
        if (includeTags.empty())
        {
            return QueryFromAllObjects(excludeSets);
        }

        const auto base = FindSmallestIncludeSet(includeTags);
        if (!base)
        {
            return {};
        }

        return QueryFromBaseSet(base->get(), includeTags, excludeSets);
    }

    /**
     * Helper function
     * Validates that a tag is non-empty.
     *
     * @param tag The tag string to validate.
     * @return An empty expected on success, or an unexpected error message if the tag is empty.
     */
    std::expected<void, TagManager::FindSetError>
    TagManager::ValidateTags(const std::vector<std::string_view>& tags) const
    {
        for (auto tag : tags)
        {
            auto result = AssertValidTag(tag);
            if (!result)
                return result;
        }
        return {};
    }

    /**
     * Helper function
     * Retrieves all existing object sets associated with the given tags.
     * Tags that do not exist in the internal map are ignored.
     *
     * @param tags The list of tags to resolve into object sets.
     * @return A vector of references to existing ObjectSets.
     */
    std::vector<std::reference_wrapper<const TagManager::ObjectSet>>
    TagManager::CollectExistingSets(const std::vector<std::string_view>& tags) const
    {
        std::vector<std::reference_wrapper<const ObjectSet>> sets;
        sets.reserve(tags.size());

        for (auto tag : tags)
        {
            auto setResult = FindSet(tag);
            if (setResult)
            {
                sets.push_back(*setResult.value());
            }
        }

        return sets;
    }

    /**
     * Helper function
     * Finds the smallest object set among the provided include tags.
     * This is used as the base set to optimize query performance.
     *
     * @param includeTags The list of required tags.
     * @return An optional reference to the smallest ObjectSet, or std::nullopt if any tag is missing.
     */
    std::optional<std::reference_wrapper<const TagManager::ObjectSet>>
    TagManager::FindSmallestIncludeSet(const std::vector<std::string_view>& includeTags) const
    {
        std::optional<std::reference_wrapper<const ObjectSet>> base;

        for (auto tag : includeTags)
        {
            auto setResult = FindSet(tag);
            if (!setResult)
            {
                return std::nullopt;
            }

            const auto& currentSet = *setResult.value();
            if (!base || currentSet.size() < base->get().size())
            {
                base = currentSet;
            }
        }

        return base;
    }

    /**
     * Helper function
     * Checks whether an object is present in any of the excluded object sets.
     *
     * @param id The object ID to check.
     * @param excludeSets The collection of object sets to exclude.
     * @return True if the object is found in any exclude set, false otherwise.
     */
    bool TagManager::IsExcluded(ObjectId id, const std::vector<std::reference_wrapper<const ObjectSet>>& excludeSets) const
    {
        for (const auto& setRef : excludeSets)
        {
            const auto& set = setRef.get();
            if (set.find(id) != set.end())
            {
                return true;
            }
        }
        return false;
    }

    /**
     * Helper function
     * Determines whether an object contains all required include tags.
     *
     * @param id The object ID to check.
     * @param includeTags The list of required tags.
     * @return True if the object contains all include tags, false otherwise.
     */
    bool TagManager::MatchesIncludeTags(ObjectId id, const std::vector<std::string_view>& includeTags) const
    {
        for (auto tag : includeTags)
        {
            if (!Contains(id, tag))
            {
                return false;
            }
        }
        return true;
    }

    /**
     * Helper function
     * Performs a query starting from all known objects, filtering out excluded ones.
     *
     * @param excludeSets The collection of object sets to exclude.
     * @return A vector of object IDs that are not excluded.
     */
    std::vector<TagManager::ObjectId>
    TagManager::QueryFromAllObjects(const std::vector<std::reference_wrapper<const ObjectSet>>& excludeSets) const
    {
        std::vector<ObjectId> out;
        out.reserve(mAllObjects.size());

        for (auto id : mAllObjects)
        {
            if (!IsExcluded(id, excludeSets))
            {
                out.push_back(id);
            }
        }

        return out;
    }

    /**
     * Helper function
     * Performs a query starting from a base object set, applying include and exclude filters.
     *
     * @param baseSet The base set of objects to iterate over.
     * @param includeTags The list of required tags.
     * @param excludeSets The collection of object sets to exclude.
     * @return A vector of object IDs that satisfy all query conditions.
     */
    std::vector<TagManager::ObjectId>
    TagManager::QueryFromBaseSet(
        const ObjectSet& baseSet,
        const std::vector<std::string_view>& includeTags,
        const std::vector<std::reference_wrapper<const ObjectSet>>& excludeSets) const
    {
        std::vector<ObjectId> out;
        out.reserve(baseSet.size());

        for (auto id : baseSet)
        {
            if (IsExcluded(id, excludeSets))
            {
                continue;
            }

            if (MatchesIncludeTags(id, includeTags))
            {
                out.push_back(id);
            }
        }

        return out;
    }

    /**
     * Checks whether an object has a specific tag.
     *
     * @param owner The object identifier.
     * @param tag The tag to check.
     * @return true if the object has the tag; false otherwise.
     *
     * @throws std::invalid_argument if the tag is empty.
     */
    bool TagManager::HasTag(ObjectId owner, std::string_view tag) const
    {
        AssertValidTag(tag);
        return Contains(owner, tag);
    }

    /**
     * Removes all tags and objects from the Tagmanager.
     */
    void TagManager::Clear()
    {
        mTagToObjects.clear();
        mAllObjects.clear();
    }

}
