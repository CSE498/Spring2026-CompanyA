#include "TagManager.hpp"

#include <cassert>
#include <utility>

namespace cse498
{
    void TagManager::AssertValidTag(std::string_view tag)
    {
        assert(!tag.empty() && "TagManager: tag must not be empty");
    }

    const TagManager::ObjectSet* TagManager::FindSet(std::string_view tag) const
    {
        auto it = mTagToObjects.find(std::string(tag));
        return (it == mTagToObjects.end()) ? nullptr : &it->second;
    }

    void TagManager::OnTagAdded(ObjectId owner, std::string_view tag)
    {
        AssertValidTag(tag);

        mAllObjects.insert(owner);

        auto& setForTag = mTagToObjects[std::string(tag)];
        setForTag.insert(owner);
    }

    void TagManager::OnTagRemoved(ObjectId owner, std::string_view tag)
    {
        AssertValidTag(tag);

        auto it = mTagToObjects.find(std::string(tag));
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

    std::vector<TagManager::ObjectId> TagManager::Query(
        const std::vector<std::string_view>& includeTags,
        const std::vector<std::string_view>& excludeTags
    ) const
    {
        auto IsExcluded = [&](ObjectId id) -> bool
        {
            for (auto t : excludeTags)
            {
                AssertValidTag(t);
                const auto* setPtr = FindSet(t);
                if (setPtr && setPtr->find(id) != setPtr->end())
                {
                    return true;
                }
            }
            return false;
        };

        std::vector<ObjectId> out;

        // Case 1: includeTags empty -> start from universe
        if (includeTags.empty())
        {
            out.reserve(mAllObjects.size());
            for (auto id : mAllObjects)
            {
                if (!IsExcluded(id)) out.push_back(id);
            }
            return out;
        }

        // Case 2: must contain ALL include tags
        const ObjectSet* baseSet = nullptr;
        std::string_view baseTag{};

        for (auto t : includeTags)
        {
            AssertValidTag(t);
            const auto* s = FindSet(t);
            if (!s)
            {
                return {}; // missing required tag
            }
            if (!baseSet || s->size() < baseSet->size())
            {
                baseSet = s;
                baseTag = t;
            }
        }

        out.reserve(baseSet->size());

        for (auto id : *baseSet)
        {
            if (IsExcluded(id)) continue;

            bool ok = true;
            for (auto t : includeTags)
            {
                if (t == baseTag) continue;
                const auto* s = FindSet(t);
                if (s->find(id) == s->end())
                {
                    ok = false;
                    break;
                }
            }
            if (ok) out.push_back(id);
        }

        return out;
    }

    bool TagManager::HasTag(ObjectId owner, std::string_view tag) const
    {
        AssertValidTag(tag);
        const auto* setPtr = FindSet(tag);
        return setPtr && (setPtr->find(owner) != setPtr->end());
    }

    void TagManager::Clear()
    {
        mTagToObjects.clear();
        mAllObjects.clear();
    }

}
