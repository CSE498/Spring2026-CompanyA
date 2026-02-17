#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/TagManager.hpp"

#include <algorithm>
#include <string_view>
#include <unordered_set>
#include <vector>

using cse498::TagManager;

// Helpers Functions
static std::unordered_set<TagManager::ObjectId>
ToSet(const std::vector<TagManager::ObjectId>& v)
{
    return { v.begin(), v.end() };
}

static std::vector<std::string_view> SV(std::initializer_list<std::string_view> tags)
{
    return std::vector<std::string_view>(tags.begin(), tags.end());
}


// Tests
TEST_CASE("TagManager: OnTagAdded makes HasTag true and Query returns owner", "[TagManager]")
{
    TagManager tm;
    TagManager::ObjectId a = 1;

    tm.OnTagAdded(a, "red");

    REQUIRE(tm.HasTag(a, "red") == true);

    auto res = tm.Query(SV({ "red" }));
    REQUIRE(ToSet(res) == std::unordered_set<TagManager::ObjectId>{a});
}

TEST_CASE("TagManager: Adding same tag twice does not duplicate results", "[TagManager]")
{
    TagManager tm;
    TagManager::ObjectId a = 1;

    tm.OnTagAdded(a, "red");
    tm.OnTagAdded(a, "red"); // duplicate add

    auto res = tm.Query(SV({ "red" }));
    REQUIRE(res.size() == 1);
    REQUIRE(res[0] == a);
}

TEST_CASE("TagManager: OnTagRemoved removes membership for that tag", "[TagManager]")
{
    TagManager tm;
    TagManager::ObjectId a = 1;

    tm.OnTagAdded(a, "red");
    tm.OnTagRemoved(a, "red");

    REQUIRE(tm.HasTag(a, "red") == false);

    auto res = tm.Query(SV({ "red" }));
    REQUIRE(res.empty());
}

TEST_CASE("TagManager: Removing a non-existent tag does not crash and keeps state", "[TagManager]")
{
    TagManager tm;
    TagManager::ObjectId a = 1;

    tm.OnTagAdded(a, "red");
    tm.OnTagRemoved(a, "blue"); // not present

    REQUIRE(tm.HasTag(a, "red") == true);

    auto res = tm.Query(SV({ "red" }));
    REQUIRE(ToSet(res) == std::unordered_set<TagManager::ObjectId>{a});
}

TEST_CASE("TagManager: Query with multiple include tags returns intersection", "[TagManager]")
{
    TagManager tm;
    TagManager::ObjectId a = 1;
    TagManager::ObjectId b = 2;
    TagManager::ObjectId c = 3;

    tm.OnTagAdded(a, "red");
    tm.OnTagAdded(a, "round");

    tm.OnTagAdded(b, "red");

    tm.OnTagAdded(c, "round");

    // Must have both "red" and "round" -> only a
    auto res = tm.Query(SV({ "red", "round" }));
    REQUIRE(ToSet(res) == std::unordered_set<TagManager::ObjectId>{a});
}

TEST_CASE("TagManager: Query returns empty if any include tag missing globally", "[TagManager]")
{
    TagManager tm;
    TagManager::ObjectId a = 1;

    tm.OnTagAdded(a, "red");

    // No one has "ghost" tag
    auto res = tm.Query(SV({ "red", "ghost" }));
    REQUIRE(res.empty());
}

TEST_CASE("TagManager: Query with excludeTags filters out matches", "[TagManager]")
{
    TagManager tm;
    TagManager::ObjectId a = 1;
    TagManager::ObjectId b = 2;

    tm.OnTagAdded(a, "red");
    tm.OnTagAdded(a, "bad");

    tm.OnTagAdded(b, "red");

    // Include "red" but exclude "bad" -> only b
    auto res = tm.Query(SV({ "red" }), SV({ "bad" }));
    REQUIRE(ToSet(res) == std::unordered_set<TagManager::ObjectId>{b});
}

TEST_CASE("TagManager: Query with empty includeTags uses universe then excludes", "[TagManager]")
{
    TagManager tm;
    TagManager::ObjectId a = 1;
    TagManager::ObjectId b = 2;
    TagManager::ObjectId c = 3;

    tm.OnTagAdded(a, "red");   // a in universe
    tm.OnTagAdded(b, "blue");  // b in universe
    tm.OnTagAdded(c, "bad");   // c in universe

    // include empty => start from universe => {a,b,c} then exclude "bad" => {a,b}
    auto res = tm.Query({}, SV({ "bad" }));
    REQUIRE(ToSet(res) == std::unordered_set<TagManager::ObjectId>{a, b});
}

TEST_CASE("TagManager: Universe keeps owners even if their last tag is removed", "[TagManager]")
{
    TagManager tm;
    TagManager::ObjectId a = 1;

    tm.OnTagAdded(a, "temp");
    tm.OnTagRemoved(a, "temp");

    // Owner should still be in universe, so Query(include empty) should return it
    auto res = tm.Query({});
    REQUIRE(ToSet(res) == std::unordered_set<TagManager::ObjectId>{a});
}

TEST_CASE("TagManager: Clear removes all tags and universe", "[TagManager]")
{
    TagManager tm;
    TagManager::ObjectId a = 1;
    TagManager::ObjectId b = 2;

    tm.OnTagAdded(a, "red");
    tm.OnTagAdded(b, "blue");

    tm.Clear();

    REQUIRE(tm.HasTag(a, "red") == false);
    REQUIRE(tm.HasTag(b, "blue") == false);

    REQUIRE(tm.Query(SV({ "red" })).empty());
    REQUIRE(tm.Query(SV({ "blue" })).empty());
    REQUIRE(tm.Query({}).empty()); // universe cleared too
}
