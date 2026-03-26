#include "TagManager.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

namespace cse498 {
/**
 * Validates that a tag is non-empty.
 *
 * In debug builds, triggers an assertion if the tag is empty. In all builds,
 * throws std::invalid_argument if the tag is empty.
 *
 * @param tag The tag string to validate.
 * @throws std::invalid_argument if the tag is empty.
 */
void TagManager::AssertValidTag(std::string_view tag) {

  if (tag.empty()) {
    throw std::invalid_argument("TagManager: tag must not be empty");
  }
}

/**
 * Finds the objects associated with a/multiple tags.
 *
 * Performs a lookup in the internal tag-to-object map.
 *
 * @param tag The tag to search for.
 * @return Pointer to the ObjectSet associated with the tag, or nullptr if the
 * tag does not exist.
 */
std::expected<const TagManager::ObjectSet *, TagManager::FindSetError>
TagManager::FindSet(std::string_view tag) const {
  if (tag.empty()) {
    return std::unexpected(FindSetError::EmptyTag);
  }
  auto it = mTagToObjects.find(tag);
  if (it == mTagToObjects.end()) {
    return std::unexpected(FindSetError::TagNotFound);
  }
  return &it->second;
}

/**
 * Checks whether an object is present in the ObjectSet for a tag. Suggested as
 * a helper function
 * @param owner The object identifier to test.
 * @param tag The tag name.
 * @return true if owner is associated with tag; false otherwise.
 */
bool TagManager::Contains(ObjectId owner, std::string_view tag) const {
  auto setResult = FindSet(tag);
  if (!setResult)
    return false;

  const ObjectSet *setPtr = *setResult;
  return setPtr->find(owner) != setPtr->end();
}

/**
 * Adds an object to a tag.
 *
 * Inserts the object into the tag's object set. If the tag does not already
 * exist, it is created. The object is also added to the universe of known
 * objects.
 *
 * @param owner The object identifier.
 * @param tag The tag to associate with the object.
 *
 * @throws std::invalid_argument if the tag is empty.
 */
void TagManager::OnTagAdded(ObjectId owner, std::string_view tag) {
  AssertValidTag(tag);

  mAllObjects.insert(owner);

  // Insertion requires owning key storage => std::string construction is
  // expected here.
  auto &setForTag = mTagToObjects[std::string(tag)];
  setForTag.insert(owner);
}

/**
 * Removes a tag from an object.
 *
 * If the tag exists, the object is removed from its associated set. If the
 * tag's set becomes empty, the tag entry is removed entirely. The object
 * remains in the global object set.
 *
 * @param owner The object identifier.
 * @param tag The tag to remove.
 *
 * @throws std::invalid_argument if the tag is empty.
 */
void TagManager::OnTagRemoved(ObjectId owner, std::string_view tag) {
  AssertValidTag(tag);

  auto it = mTagToObjects.find(tag);
  if (it == mTagToObjects.end()) {
    return;
  }

  it->second.erase(owner);

  // Remove empty tag buckets
  if (it->second.empty()) {
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
TagManager::Query(const std::vector<std::string_view> &includeTags,
                  const std::vector<std::string_view> &excludeTags) const {
  // Validate tags once
  for (auto tag : includeTags)
    AssertValidTag(tag);
  for (auto tag : excludeTags)
    AssertValidTag(tag);

  // Cache exclude sets once (avoids repeated map lookups)
  std::vector<const ObjectSet *> excludeSets;
  excludeSets.reserve(excludeTags.size());
  for (auto tag : excludeTags) {
    auto setResult = FindSet(tag);
    excludeSets.push_back(setResult ? *setResult : nullptr);
  } // may be nullptr if tag not present

  auto IsExcluded = [&](ObjectId id) -> bool {
    for (const auto *s : excludeSets) {
      if (s && s->find(id) != s->end()) {
        return true;
      }
    }
    return false;
  };

  std::vector<ObjectId> out;

  // Case 1: No include tags -> start from universe
  if (includeTags.empty()) {
    out.reserve(mAllObjects.size());
    for (auto id : mAllObjects) {
      if (!IsExcluded(id)) {
        out.push_back(id);
      }
    }
    return out;
  }

  // Case 2: Must contain ALL include tags
  const ObjectSet *baseSet = nullptr;
  std::string_view baseTag{};

  // Pick the smallest include set as the base (fastest iteration)
  for (auto tag : includeTags) {
    auto setResult = FindSet(tag);
    if (!setResult) {
      return {}; // missing required tag => no results
    }

    const ObjectSet *currentSet = *setResult;
    if (!baseSet || currentSet->size() < baseSet->size()) {
      baseSet = currentSet;
      baseTag = tag;
    }
  }

  assert(baseSet != nullptr && "Query: baseSet should not be null here");
  out.reserve(baseSet->size());

  for (auto id : *baseSet) {
    if (IsExcluded(id)) {
      continue;
    }
    bool matchesAllTags = true;
    for (auto tag : includeTags) {
      if (tag == baseTag) {
        continue;
      }
      if (!Contains(id, tag)) {
        matchesAllTags = false;
        break;
      }
    }

    if (matchesAllTags) {
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
bool TagManager::HasTag(ObjectId owner, std::string_view tag) const {
  AssertValidTag(tag);
  return Contains(owner, tag);
}

/**
 * Removes all tags and objects from the Tagmanager.
 */
void TagManager::Clear() {
  mTagToObjects.clear();
  mAllObjects.clear();
}

} // namespace cse498
