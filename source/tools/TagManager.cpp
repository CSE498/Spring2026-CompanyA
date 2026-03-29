#include "TagManager.hpp"

namespace cse498 {

bool TagManager::AssertValidTag(std::string_view tag) { return !tag.empty(); }

bool TagManager::ValidateTags(const std::vector<std::string_view> &tags) const {
  for (const auto tag : tags) {
    if (!AssertValidTag(tag)) {
      return false;
    }
  }

  return true;
}

const TagManager::ObjectSet *TagManager::FindSet(std::string_view tag) const {
  if (!AssertValidTag(tag)) {
    return nullptr;
  }

  auto it = mTagToObjects.find(std::string(tag));
  if (it == mTagToObjects.end()) {
    return nullptr;
  }

  return &it->second;
}

bool TagManager::Contains(ObjectId owner, std::string_view tag) const {
  const ObjectSet *setPtr = FindSet(tag);
  if (setPtr == nullptr) {
    return false;
  }

  return setPtr->find(owner) != setPtr->end();
}

void TagManager::OnTagAdded(ObjectId owner, std::string_view tag) {
  if (!AssertValidTag(tag)) {
    return;
  }

  mAllObjects.insert(owner);
  mTagToObjects[std::string(tag)].insert(owner);
}

void TagManager::OnTagRemoved(ObjectId owner, std::string_view tag) {
  if (!AssertValidTag(tag)) {
    return;
  }

  auto it = mTagToObjects.find(std::string(tag));
  if (it == mTagToObjects.end()) {
    return;
  }

  it->second.erase(owner);

  if (it->second.empty()) {
    mTagToObjects.erase(it);
  }
}

std::vector<const TagManager::ObjectSet *> TagManager::CollectExistingSets(
    const std::vector<std::string_view> &tags) const {
  std::vector<const ObjectSet *> sets;
  sets.reserve(tags.size());

  for (const auto tag : tags) {
    const ObjectSet *setPtr = FindSet(tag);
    if (setPtr != nullptr) {
      sets.push_back(setPtr);
    }
  }

  return sets;
}

const TagManager::ObjectSet *TagManager::FindSmallestIncludeSet(
    const std::vector<std::string_view> &includeTags) const {
  const ObjectSet *base = nullptr;

  for (const auto tag : includeTags) {
    const ObjectSet *setPtr = FindSet(tag);
    if (setPtr == nullptr) {
      return nullptr;
    }

    if (base == nullptr || setPtr->size() < base->size()) {
      base = setPtr;
    }
  }

  return base;
}

bool TagManager::IsExcluded(
    ObjectId id, const std::vector<const ObjectSet *> &excludeSets) const {
  for (const ObjectSet *setPtr : excludeSets) {
    if (setPtr->find(id) != setPtr->end()) {
      return true;
    }
  }

  return false;
}

bool TagManager::MatchesIncludeTags(
    ObjectId id, const std::vector<std::string_view> &includeTags) const {
  for (const auto tag : includeTags) {
    if (!Contains(id, tag)) {
      return false;
    }
  }

  return true;
}

std::vector<TagManager::ObjectId> TagManager::QueryFromAllObjects(
    const std::vector<const ObjectSet *> &excludeSets) const {
  std::vector<ObjectId> out;
  out.reserve(mAllObjects.size());

  for (const auto id : mAllObjects) {
    if (!IsExcluded(id, excludeSets)) {
      out.push_back(id);
    }
  }

  return out;
}

std::vector<TagManager::ObjectId> TagManager::QueryFromBaseSet(
    const ObjectSet &baseSet, const std::vector<std::string_view> &includeTags,
    const std::vector<const ObjectSet *> &excludeSets) const {
  std::vector<ObjectId> out;
  out.reserve(baseSet.size());

  for (const auto id : baseSet) {
    if (IsExcluded(id, excludeSets)) {
      continue;
    }

    if (MatchesIncludeTags(id, includeTags)) {
      out.push_back(id);
    }
  }

  return out;
}

std::vector<TagManager::ObjectId>
TagManager::Query(const std::vector<std::string_view> &includeTags,
                  const std::vector<std::string_view> &excludeTags) const {
  if (!ValidateTags(includeTags)) {
    return {};
  }

  if (!ValidateTags(excludeTags)) {
    return {};
  }

  const auto excludeSets = CollectExistingSets(excludeTags);

  if (includeTags.empty()) {
    return QueryFromAllObjects(excludeSets);
  }

  const ObjectSet *base = FindSmallestIncludeSet(includeTags);
  if (base == nullptr) {
    return {};
  }

  return QueryFromBaseSet(*base, includeTags, excludeSets);
}

bool TagManager::HasTag(ObjectId owner, std::string_view tag) const {
  if (!AssertValidTag(tag)) {
    return false;
  }

  return Contains(owner, tag);
}

void TagManager::Clear() {
  mTagToObjects.clear();
  mAllObjects.clear();
}

} // namespace cse498