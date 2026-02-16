// tests/mocks/TagManager.h
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// Mock class for testing purposes
namespace cse498 {
    class TagManager {
    public:
    struct Call {
        std::uint64_t owner{};
        std::string tag;
    };

    std::vector<Call> added;
    std::vector<Call> removed;

    void OnTagAdded(std::uint64_t owner, std::string_view tag) noexcept {
        added.push_back(Call{owner, std::string(tag)});
    }

    void OnTagRemoved(std::uint64_t owner, std::string_view tag) noexcept {
        removed.push_back(Call{owner, std::string(tag)});
    }

    void ClearCalls() {
        added.clear();
        removed.clear();
    }
    };
}

// NOTE: CATCH_CONFIG_MAIN is defined elsewhere in your test suite.
//       do NOT define it again here.
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

// Use your actual project include path here:
#include "../../source/tools/AnnotationSet.hpp"

// If your AnnotationSet.cpp includes "TagManager.h", make sure your build
// can find the mock TagManager.h above (tests/mocks/TagManager.h) OR
// your real TagManager.h.
//
// Most build systems do this by adding `tests/mocks` to include directories
// before `source/...` include directories.

#include <algorithm>
#include <set>
#include <string>
#include <vector>

// ====================================================================
//  1. Construction / basic accessors
// ====================================================================

TEST_CASE("Construction stores owner and starts empty", "[agents][AnnotationSet]")
{
  cse498::AnnotationSet set(123);
  CHECK(set.Owner() == 123);
  CHECK(set.Size() == 0);
  CHECK(set.Empty());
}

TEST_CASE("Construction with TagManager does not notify immediately", "[agents][AnnotationSet]")
{
  cse498::TagManager mgr;
  cse498::AnnotationSet set(7, &mgr);

  CHECK(mgr.added.empty());
  CHECK(mgr.removed.empty());
  CHECK(set.Owner() == 7);
  CHECK(set.Empty());
}

TEST_CASE("AttachManager sets manager pointer", "[agents][AnnotationSet]")
{
  cse498::TagManager mgr;
  cse498::AnnotationSet set(42);

  // No manager, so no notifications
  CHECK(set.AddTag("alpha"));
  CHECK(mgr.added.empty());

  // Attach manager, future updates should notify
  set.AttachManager(&mgr);
  CHECK(set.AddTag("beta"));
  REQUIRE(mgr.added.size() == 1);
  CHECK(mgr.added[0].owner == 42);
  CHECK(mgr.added[0].tag == "beta");
}

// ====================================================================
//  2. AddTag / HasTag
// ====================================================================

TEST_CASE("AddTag inserts and HasTag finds it", "[agents][AnnotationSet]")
{
  cse498::AnnotationSet set(1);

  CHECK(set.AddTag("hostile"));
  CHECK_FALSE(set.Empty());
  CHECK(set.Size() == 1);
  CHECK(set.HasTag("hostile"));
  CHECK_FALSE(set.HasTag("friendly"));
}

TEST_CASE("AddTag returns false on duplicate and does not change size", "[agents][AnnotationSet]")
{
  cse498::AnnotationSet set(1);

  CHECK(set.AddTag("seen_before"));
  CHECK_FALSE(set.AddTag("seen_before")); // duplicate
  CHECK(set.Size() == 1);
  CHECK(set.HasTag("seen_before"));
}

TEST_CASE("AddTag multiple distinct tags", "[agents][AnnotationSet]")
{
  cse498::AnnotationSet set(1);

  CHECK(set.AddTag("a"));
  CHECK(set.AddTag("b"));
  CHECK(set.AddTag("c"));
  CHECK(set.Size() == 3);

  CHECK(set.HasTag("a"));
  CHECK(set.HasTag("b"));
  CHECK(set.HasTag("c"));
  CHECK_FALSE(set.HasTag("d"));
}

// ====================================================================
//  3. RemoveTag
// ====================================================================

TEST_CASE("RemoveTag removes existing tag", "[agents][AnnotationSet]")
{
  cse498::AnnotationSet set(1);

  set.AddTag("x");
  set.AddTag("y");
  set.AddTag("z");

  CHECK(set.RemoveTag("y"));
  CHECK(set.Size() == 2);
  CHECK_FALSE(set.HasTag("y"));
  CHECK(set.HasTag("x"));
  CHECK(set.HasTag("z"));
}

TEST_CASE("RemoveTag returns false for missing tag", "[agents][AnnotationSet]")
{
  cse498::AnnotationSet set(1);

  set.AddTag("x");
  CHECK_FALSE(set.RemoveTag("missing"));
  CHECK(set.Size() == 1);
  CHECK(set.HasTag("x"));
}

TEST_CASE("RemoveTag then re-add same tag works", "[agents][AnnotationSet]")
{
  cse498::AnnotationSet set(1);

  CHECK(set.AddTag("loop"));
  CHECK(set.RemoveTag("loop"));
  CHECK_FALSE(set.HasTag("loop"));
  CHECK(set.AddTag("loop"));
  CHECK(set.HasTag("loop"));
  CHECK(set.Size() == 1);
}

// ====================================================================
//  4. Clear
// ====================================================================

TEST_CASE("Clear on set without manager removes all tags", "[agents][AnnotationSet]")
{
  cse498::AnnotationSet set(99);
  set.AddTag("a");
  set.AddTag("b");
  set.AddTag("c");
  CHECK(set.Size() == 3);

  set.Clear();
  CHECK(set.Size() == 0);
  CHECK(set.Empty());
  CHECK_FALSE(set.HasTag("a"));
  CHECK_FALSE(set.HasTag("b"));
  CHECK_FALSE(set.HasTag("c"));
}

TEST_CASE("Clear then reuse the set", "[agents][AnnotationSet]")
{
  cse498::AnnotationSet set(99);
  set.AddTag("a");
  set.AddTag("b");
  set.Clear();

  CHECK(set.Empty());
  CHECK(set.AddTag("new"));
  CHECK(set.Size() == 1);
  CHECK(set.HasTag("new"));
  CHECK_FALSE(set.HasTag("a"));
}

// ====================================================================
//  5. ToVector
// ====================================================================

TEST_CASE("ToVector returns all tags (order-independent)", "[agents][AnnotationSet]")
{
  cse498::AnnotationSet set(5);
  set.AddTag("alpha");
  set.AddTag("beta");
  set.AddTag("gamma");

  auto vec = set.ToVector();
  CHECK(vec.size() == 3);

  std::set<std::string> got(vec.begin(), vec.end());
  std::set<std::string> expected = {"alpha", "beta", "gamma"};
  CHECK(got == expected);
}

// ====================================================================
//  6. Iteration (range-for)
// ====================================================================

TEST_CASE("Range-for visits all tags (order-independent)", "[agents][AnnotationSet]")
{
  cse498::AnnotationSet set(5);
  std::set<std::string> expected = {"t1", "t2", "t3", "t4"};

  for (const auto& t : expected) {
    set.AddTag(t);
  }

  std::set<std::string> visited;
  for (const auto& t : set) {
    visited.insert(t);
  }

  CHECK(visited == expected);
}

TEST_CASE("Iteration over empty set", "[agents][AnnotationSet]")
{
  cse498::AnnotationSet set(5);
  int count = 0;
  for ([[maybe_unused]] const auto& t : set) {
    ++count;
  }
  CHECK(count == 0);
}

// ====================================================================
//  7. TagManager notifications
// ====================================================================

TEST_CASE("AddTag notifies TagManager once per successful insert", "[agents][AnnotationSet]")
{
  cse498::TagManager mgr;
  cse498::AnnotationSet set(777, &mgr);

  CHECK(set.AddTag("a"));
  CHECK(set.AddTag("b"));
  CHECK_FALSE(set.AddTag("a")); // duplicate: should NOT notify

  REQUIRE(mgr.added.size() == 2);
  CHECK(mgr.added[0].owner == 777);
  CHECK(mgr.added[1].owner == 777);

  std::set<std::string> tags = {mgr.added[0].tag, mgr.added[1].tag};
  CHECK(tags == std::set<std::string>{"a", "b"});
}

TEST_CASE("RemoveTag notifies TagManager only when removal succeeds", "[agents][AnnotationSet]")
{
  cse498::TagManager mgr;
  cse498::AnnotationSet set(10, &mgr);

  set.AddTag("x");
  set.AddTag("y");

  mgr.ClearCalls();

  CHECK(set.RemoveTag("x"));
  CHECK_FALSE(set.RemoveTag("missing")); // should not notify
  CHECK(set.RemoveTag("y"));

  REQUIRE(mgr.removed.size() == 2);
  CHECK(mgr.removed[0].owner == 10);
  CHECK(mgr.removed[1].owner == 10);

  std::set<std::string> tags = {mgr.removed[0].tag, mgr.removed[1].tag};
  CHECK(tags == std::set<std::string>{"x", "y"});
}

TEST_CASE("Clear notifies TagManager once per existing tag", "[agents][AnnotationSet]")
{
  cse498::TagManager mgr;
  cse498::AnnotationSet set(55, &mgr);

  set.AddTag("a");
  set.AddTag("b");
  set.AddTag("c");

  mgr.ClearCalls();

  set.Clear();
  CHECK(set.Empty());

  REQUIRE(mgr.removed.size() == 3);
  for (const auto& call : mgr.removed) {
    CHECK(call.owner == 55);
  }

  std::set<std::string> tags;
  for (const auto& call : mgr.removed) tags.insert(call.tag);
  CHECK(tags == std::set<std::string>{"a", "b", "c"});
}

// ====================================================================
//  8. Larger insert/remove stress sanity
// ====================================================================

TEST_CASE("Insert many tags and remove half", "[agents][AnnotationSet]")
{
  cse498::AnnotationSet set(1);

  const int N = 2000;
  for (int i = 0; i < N; ++i) {
    CHECK(set.AddTag("tag_" + std::to_string(i)));
  }
  CHECK(set.Size() == static_cast<size_t>(N));

  // Remove evens
  for (int i = 0; i < N; i += 2) {
    CHECK(set.RemoveTag("tag_" + std::to_string(i)));
  }
  CHECK(set.Size() == static_cast<size_t>(N / 2));

  // Verify odds remain, evens gone
  for (int i = 0; i < N; ++i) {
    if (i % 2 == 0) {
      CHECK_FALSE(set.HasTag("tag_" + std::to_string(i)));
    } else {
      CHECK(set.HasTag("tag_" + std::to_string(i)));
    }
  }
}
