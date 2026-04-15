#include "../../source/tools/AnnotationSet.hpp"
#include <set>
#include <string>
#include <vector>
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

static std::set<std::string> AsSet(const std::vector<std::string> &v) {
    return std::set<std::string>(v.begin(), v.end());
}

// ====================================================================
//  1) Basics + add/has/remove + ToVector + iteration
// ====================================================================

TEST_CASE("AnnotationSet basic behavior", "[tools][AnnotationSet]") {
    cse498::AnnotationSet set(123);

    CHECK(set.Owner() == 123);
    CHECK(set.Empty());
    CHECK(set.Size() == 0);

    // Add / Has / duplicates
    CHECK(set.AddTag("a"));
    CHECK(set.AddTag("b"));
    CHECK_FALSE(set.AddTag("a")); // duplicate
    CHECK(set.Size() == 2);
    CHECK_FALSE(set.Empty());

    CHECK(set.HasTag("a"));
    CHECK(set.HasTag("b"));
    CHECK_FALSE(set.HasTag("c"));

    // Remove
    CHECK(set.RemoveTag("a"));
    CHECK_FALSE(set.RemoveTag("a")); // already removed
    CHECK(set.Size() == 1);
    CHECK_FALSE(set.HasTag("a"));
    CHECK(set.HasTag("b"));

    // ToVector (order independent)
    CHECK(AsSet(set.ToVector()) == std::set<std::string>{"b"});

    // Iteration (order independent)
    std::set<std::string> visited;
    for (const auto &t: set)
        visited.insert(t);
    CHECK(visited == std::set<std::string>{"b"});
}

// ====================================================================
//  2) Clear + reuse
// ====================================================================

TEST_CASE("AnnotationSet Clear removes all tags and can be reused", "[tools][AnnotationSet]") {
    cse498::AnnotationSet set(99);

    CHECK(set.AddTag("x"));
    CHECK(set.AddTag("y"));
    CHECK(set.Size() == 2);

    set.Clear();
    CHECK(set.Empty());
    CHECK(set.Size() == 0);
    CHECK_FALSE(set.HasTag("x"));
    CHECK_FALSE(set.HasTag("y"));

    // Reuse after clear
    CHECK(set.AddTag("z"));
    CHECK(set.Size() == 1);
    CHECK(set.HasTag("z"));
}

// ====================================================================
//  3) Small stress sanity
// ====================================================================

TEST_CASE("AnnotationSet handles many tags sanity", "[tools][AnnotationSet]") {
    cse498::AnnotationSet set(1);

    const int N = 200; // enough to exercise growth, fast to run
    for (int i = 0; i < N; ++i) {
        CHECK(set.AddTag("t" + std::to_string(i)));
    }
    CHECK(set.Size() == static_cast<size_t>(N));

    // Remove evens
    for (int i = 0; i < N; i += 2) {
        CHECK(set.RemoveTag("t" + std::to_string(i)));
    }
    CHECK(set.Size() == static_cast<size_t>(N / 2));

    // Verify odds remain, evens gone
    for (int i = 0; i < N; ++i) {
        if (i % 2 == 0)
            CHECK_FALSE(set.HasTag("t" + std::to_string(i)));
        else
            CHECK(set.HasTag("t" + std::to_string(i)));
    }
}
