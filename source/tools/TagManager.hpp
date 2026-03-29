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

  void OnTagAdded(ObjectId owner, std::string_view tag);
  void OnTagRemoved(ObjectId owner, std::string_view tag);

  std::vector<ObjectId>
  Query(const std::vector<std::string_view> &includeTags,
        const std::vector<std::string_view> &excludeTags = {}) const;

  bool HasTag(ObjectId owner, std::string_view tag) const;
  void Clear();

private:
  using ObjectSet = std::unordered_set<ObjectId>;
  using TagMap = std::unordered_map<std::string, ObjectSet>;

  static bool AssertValidTag(std::string_view tag);
  bool ValidateTags(const std::vector<std::string_view> &tags) const;
  const ObjectSet *FindSet(std::string_view tag) const;

  std::vector<const ObjectSet *>
  CollectExistingSets(const std::vector<std::string_view> &tags) const;

  const ObjectSet *FindSmallestIncludeSet(
      const std::vector<std::string_view> &includeTags) const;

  bool IsExcluded(ObjectId id,
                  const std::vector<const ObjectSet *> &excludeSets) const;

  bool
  MatchesIncludeTags(ObjectId id,
                     const std::vector<std::string_view> &includeTags) const;

  std::vector<ObjectId>
  QueryFromAllObjects(const std::vector<const ObjectSet *> &excludeSets) const;

  std::vector<ObjectId>
  QueryFromBaseSet(const ObjectSet &baseSet,
                   const std::vector<std::string_view> &includeTags,
                   const std::vector<const ObjectSet *> &excludeSets) const;

  bool Contains(ObjectId owner, std::string_view tag) const;

  TagMap mTagToObjects;
  ObjectSet mAllObjects;
};

} // namespace cse498