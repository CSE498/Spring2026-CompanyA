// NOTE: CATCH_CONFIG_MAIN is defined in tests/core/WorldPosition.cpp;
//       do NOT define it again here.
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/RobinHoodMap.hpp"

#include <string>
#include <vector>
#include <set>
#include <algorithm>

// ====================================================================
//  1. Construction
// ====================================================================

TEST_CASE("Default construction", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  CHECK(map.Size() == 0);
  CHECK(map.Empty());
  CHECK(map.BucketCount() == 16);  // default capacity
}

TEST_CASE("Construction with custom capacity", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map(64);
  CHECK(map.Size() == 0);
  CHECK(map.Empty());
  CHECK(map.BucketCount() == 64);
}

TEST_CASE("Construction with custom load factor", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map(32, 0.5);
  CHECK(map.GetMaxLoadFactor() == 0.5);
}

TEST_CASE("Initializer list construction", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<std::string, int> map = {
    {"alpha", 1}, {"beta", 2}, {"gamma", 3}
  };
  CHECK(map.Size() == 3);
  CHECK_FALSE(map.Empty());
  CHECK(map.Contains("alpha"));
  CHECK(map.Contains("beta"));
  CHECK(map.Contains("gamma"));
  CHECK(*map.Find("alpha") == 1);
  CHECK(*map.Find("beta")  == 2);
  CHECK(*map.Find("gamma") == 3);
}

// ====================================================================
//  2. Basic Insert and Find
// ====================================================================

TEST_CASE("Insert single element and find it", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, std::string> map;
  bool inserted = map.Insert(42, "hello");
  CHECK(inserted);
  CHECK(map.Size() == 1);

  std::string * val = map.Find(42);
  REQUIRE(val != nullptr);
  CHECK(*val == "hello");
}

TEST_CASE("Find returns nullptr for missing key", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  map.Insert(1, 100);
  CHECK(map.Find(1) != nullptr);
  CHECK(map.Find(2) == nullptr);
  CHECK(map.Find(999) == nullptr);
}

TEST_CASE("Insert multiple elements", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<std::string, int> map;
  map.Insert("a", 1);
  map.Insert("b", 2);
  map.Insert("c", 3);
  map.Insert("d", 4);
  map.Insert("e", 5);

  CHECK(map.Size() == 5);
  CHECK(*map.Find("a") == 1);
  CHECK(*map.Find("b") == 2);
  CHECK(*map.Find("c") == 3);
  CHECK(*map.Find("d") == 4);
  CHECK(*map.Find("e") == 5);
}

// ====================================================================
//  3. Insert update (same key, new value)
// ====================================================================

TEST_CASE("Insert returns false and updates on duplicate key", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  CHECK(map.Insert(1, 100) == true);   // new
  CHECK(map.Insert(1, 200) == false);  // update
  CHECK(map.Size() == 1);             // still one element
  CHECK(*map.Find(1) == 200);         // value was updated
}

TEST_CASE("Repeated updates to the same key", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<std::string, int> map;
  for (int i = 0; i < 100; ++i) {
    map.Insert("key", i);
  }
  CHECK(map.Size() == 1);
  CHECK(*map.Find("key") == 99);
}

// ====================================================================
//  4. Contains
// ====================================================================

TEST_CASE("Contains returns correct results", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  CHECK_FALSE(map.Contains(0));

  map.Insert(10, 100);
  map.Insert(20, 200);
  map.Insert(30, 300);

  CHECK(map.Contains(10));
  CHECK(map.Contains(20));
  CHECK(map.Contains(30));
  CHECK_FALSE(map.Contains(40));
  CHECK_FALSE(map.Contains(0));
  CHECK_FALSE(map.Contains(-1));
}

// ====================================================================
//  5. Erase
// ====================================================================

TEST_CASE("Erase existing key", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  map.Insert(1, 10);
  map.Insert(2, 20);
  map.Insert(3, 30);

  CHECK(map.Erase(2) == true);
  CHECK(map.Size() == 2);
  CHECK_FALSE(map.Contains(2));
  CHECK(map.Find(2) == nullptr);

  // Other elements still intact.
  CHECK(*map.Find(1) == 10);
  CHECK(*map.Find(3) == 30);
}

TEST_CASE("Erase non-existing key returns false", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  map.Insert(1, 10);
  CHECK(map.Erase(999) == false);
  CHECK(map.Size() == 1);
}

TEST_CASE("Erase all elements one by one", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  for (int i = 0; i < 20; ++i) {
    map.Insert(i, i * 10);
  }
  CHECK(map.Size() == 20);

  for (int i = 0; i < 20; ++i) {
    CHECK(map.Erase(i) == true);
  }
  CHECK(map.Size() == 0);
  CHECK(map.Empty());

  // All gone.
  for (int i = 0; i < 20; ++i) {
    CHECK_FALSE(map.Contains(i));
  }
}

TEST_CASE("Erase then re-insert the same key", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<std::string, int> map;
  map.Insert("x", 1);
  CHECK(map.Erase("x"));
  CHECK_FALSE(map.Contains("x"));

  map.Insert("x", 2);
  CHECK(map.Contains("x"));
  CHECK(*map.Find("x") == 2);
}

// ====================================================================
//  6. operator[]
// ====================================================================

TEST_CASE("operator[] reads existing value", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<std::string, int> map;
  map.Insert("key", 42);
  CHECK(map["key"] == 42);
}

TEST_CASE("operator[] auto-inserts default value", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<std::string, int> map;
  int & val = map["new_key"];  // should default-insert 0
  CHECK(val == 0);
  CHECK(map.Size() == 1);
  CHECK(map.Contains("new_key"));
}

TEST_CASE("operator[] assignment", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, std::string> map;
  map[1] = "one";
  map[2] = "two";
  map[3] = "three";

  CHECK(map.Size() == 3);
  CHECK(map[1] == "one");
  CHECK(map[2] == "two");
  CHECK(map[3] == "three");

  // Overwrite via operator[]
  map[2] = "TWO";
  CHECK(map[2] == "TWO");
  CHECK(map.Size() == 3);
}

// ====================================================================
//  7. Size and Empty
// ====================================================================

TEST_CASE("Size tracks insertions and erasures", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  CHECK(map.Size() == 0);
  CHECK(map.Empty());

  map.Insert(1, 1);
  CHECK(map.Size() == 1);
  CHECK_FALSE(map.Empty());

  map.Insert(2, 2);
  map.Insert(3, 3);
  CHECK(map.Size() == 3);

  map.Erase(2);
  CHECK(map.Size() == 2);

  map.Erase(1);
  map.Erase(3);
  CHECK(map.Size() == 0);
  CHECK(map.Empty());
}

// ====================================================================
//  8. Clear
// ====================================================================

TEST_CASE("Clear removes all elements", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  for (int i = 0; i < 50; ++i) {
    map.Insert(i, i);
  }
  CHECK(map.Size() == 50);

  map.Clear();
  CHECK(map.Size() == 0);
  CHECK(map.Empty());

  // Nothing should be found.
  for (int i = 0; i < 50; ++i) {
    CHECK_FALSE(map.Contains(i));
  }
}

TEST_CASE("Clear then reuse the map", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  map.Insert(1, 10);
  map.Insert(2, 20);
  map.Clear();

  map.Insert(3, 30);
  CHECK(map.Size() == 1);
  CHECK(*map.Find(3) == 30);
  CHECK_FALSE(map.Contains(1));
  CHECK_FALSE(map.Contains(2));
}

// ====================================================================
//  9. Iteration (range-for)
// ====================================================================

TEST_CASE("Range-for visits all elements", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  std::set<int> expected_keys = {10, 20, 30, 40, 50};
  for (int k : expected_keys) {
    map.Insert(k, k * 100);
  }

  std::set<int> visited_keys;
  for (auto & [key, value] : map) {
    visited_keys.insert(key);
    CHECK(value == key * 100);
  }
  CHECK(visited_keys == expected_keys);
}

TEST_CASE("Iteration over empty map", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  int count = 0;
  for ([[maybe_unused]] auto & [k, v] : map) {
    ++count;
  }
  CHECK(count == 0);
}

TEST_CASE("Iterator pre-increment and post-increment", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  map.Insert(1, 10);
  map.Insert(2, 20);

  // Pre-increment
  auto it = map.begin();
  auto & ref = ++it;
  CHECK(&ref == &it);  // returns reference to itself

  // Post-increment
  it = map.begin();
  auto old = it++;
  CHECK(old != it);
}

// ====================================================================
//  10. Const correctness
// ====================================================================

TEST_CASE("Const Find returns const pointer", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  map.Insert(1, 100);

  const auto & cmap = map;
  const int * val = cmap.Find(1);
  REQUIRE(val != nullptr);
  CHECK(*val == 100);
  CHECK(cmap.Find(999) == nullptr);
}

TEST_CASE("Const Contains and Size", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<std::string, double> map;
  map.Insert("pi", 3.14);

  const auto & cmap = map;
  CHECK(cmap.Contains("pi"));
  CHECK_FALSE(cmap.Contains("e"));
  CHECK(cmap.Size() == 1);
  CHECK_FALSE(cmap.Empty());
}

TEST_CASE("Const iteration with cbegin/cend", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  map.Insert(1, 10);
  map.Insert(2, 20);

  int count = 0;
  for (auto it = map.cbegin(); it != map.cend(); ++it) {
    CHECK(it->second == it->first * 10);
    ++count;
  }
  CHECK(count == 2);
}

TEST_CASE("Range-for on const map", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  map.Insert(5, 50);
  map.Insert(6, 60);

  const auto & cmap = map;
  int sum = 0;
  for (const auto & [k, v] : cmap) {
    sum += v;
  }
  CHECK(sum == 110);
}

// ====================================================================
//  11. Reserve
// ====================================================================

TEST_CASE("Reserve increases bucket count", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  size_t old_buckets = map.BucketCount();
  map.Reserve(1000);
  CHECK(map.BucketCount() > old_buckets);
  CHECK(map.Size() == 0);
}

TEST_CASE("Reserve does not shrink", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map(128);
  size_t old_buckets = map.BucketCount();
  map.Reserve(5);  // already have room
  CHECK(map.BucketCount() == old_buckets);
}

TEST_CASE("Insertions after Reserve stay within load factor", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  map.Reserve(100);
  size_t bucket_count_after_reserve = map.BucketCount();

  for (int i = 0; i < 100; ++i) {
    map.Insert(i, i);
  }
  // Should not have rehashed again since we reserved enough.
  CHECK(map.BucketCount() == bucket_count_after_reserve);
  CHECK(map.Size() == 100);
}

// ====================================================================
//  12. Load factor
// ====================================================================

TEST_CASE("Load factor increases with insertions", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map(100);
  CHECK(map.GetLoadFactor() == 0.0);

  map.Insert(1, 1);
  CHECK(map.GetLoadFactor() > 0.0);
  CHECK(map.GetLoadFactor() < 1.0);
}

TEST_CASE("SetMaxLoadFactor changes the threshold", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  map.SetMaxLoadFactor(0.5);
  CHECK(map.GetMaxLoadFactor() == 0.5);
}

// ====================================================================
//  13. Many elements (stress test)
// ====================================================================

TEST_CASE("Insert and find 10000 elements", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  const int N = 10000;

  for (int i = 0; i < N; ++i) {
    map.Insert(i, i * 3);
  }
  CHECK(map.Size() == static_cast<size_t>(N));

  // Every element should be findable with the correct value.
  for (int i = 0; i < N; ++i) {
    int * val = map.Find(i);
    REQUIRE(val != nullptr);
    CHECK(*val == i * 3);
  }

  // Keys that were never inserted should not be found.
  CHECK(map.Find(N) == nullptr);
  CHECK(map.Find(N + 1) == nullptr);
}

TEST_CASE("Insert, erase half, verify remainder (1000 elements)", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  const int N = 1000;

  for (int i = 0; i < N; ++i) {
    map.Insert(i, i);
  }

  // Erase all even keys.
  for (int i = 0; i < N; i += 2) {
    CHECK(map.Erase(i));
  }
  CHECK(map.Size() == static_cast<size_t>(N / 2));

  // Even keys should be gone; odd keys should remain.
  for (int i = 0; i < N; ++i) {
    if (i % 2 == 0) {
      CHECK_FALSE(map.Contains(i));
    } else {
      REQUIRE(map.Contains(i));
      CHECK(*map.Find(i) == i);
    }
  }
}

// ====================================================================
//  14. String keys (common real-world usage)
// ====================================================================

TEST_CASE("String keys with various operations", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<std::string, std::string> map;
  map.Insert("name", "Robin");
  map.Insert("type", "HashMap");
  map.Insert("style", "OpenAddressing");

  CHECK(map.Size() == 3);
  CHECK(*map.Find("name") == "Robin");
  CHECK(*map.Find("type") == "HashMap");
  CHECK(*map.Find("style") == "OpenAddressing");

  map.Erase("type");
  CHECK(map.Size() == 2);
  CHECK(map.Find("type") == nullptr);
  CHECK(*map.Find("name") == "Robin");
}

TEST_CASE("Many string key insertions", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<std::string, int> map;
  for (int i = 0; i < 500; ++i) {
    map.Insert("key_" + std::to_string(i), i);
  }
  CHECK(map.Size() == 500);

  for (int i = 0; i < 500; ++i) {
    REQUIRE(map.Contains("key_" + std::to_string(i)));
    CHECK(*map.Find("key_" + std::to_string(i)) == i);
  }
}

// ====================================================================
//  15. Collision handling (force many collisions)
// ====================================================================

TEST_CASE("Elements with colliding hashes are all retrievable", "[tools][RobinHoodMap]")
{
  // Use a small capacity to force heavy collisions.
  cse498::RobinHoodMap<int, int> map(4, 0.95);

  // Insert enough elements to guarantee many collisions in 4 buckets.
  for (int i = 0; i < 3; ++i) {
    map.Insert(i, i * 10);
  }

  for (int i = 0; i < 3; ++i) {
    int * val = map.Find(i);
    REQUIRE(val != nullptr);
    CHECK(*val == i * 10);
  }
}

TEST_CASE("Heavy collision with tiny initial capacity", "[tools][RobinHoodMap]")
{
  // Start with just 2 buckets — forces immediate rehashing.
  cse498::RobinHoodMap<int, int> map(2);
  for (int i = 0; i < 100; ++i) {
    map.Insert(i, i);
  }
  CHECK(map.Size() == 100);

  for (int i = 0; i < 100; ++i) {
    CHECK(map.Contains(i));
  }
}

// ====================================================================
//  16. Erase under heavy load
// ====================================================================

TEST_CASE("Erase under heavy load preserves remaining elements", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map(8);
  // Fill it up close to capacity.
  for (int i = 0; i < 50; ++i) {
    map.Insert(i, i);
  }

  // Erase every third element.
  for (int i = 0; i < 50; i += 3) {
    map.Erase(i);
  }

  // Verify the rest are still correct.
  for (int i = 0; i < 50; ++i) {
    if (i % 3 == 0) {
      CHECK_FALSE(map.Contains(i));
    } else {
      REQUIRE(map.Contains(i));
      CHECK(*map.Find(i) == i);
    }
  }
}

TEST_CASE("Interleaved insert and erase", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;

  // Insert 0-99, then erase 0-49, then insert 100-149.
  for (int i = 0; i < 100; ++i) map.Insert(i, i);
  for (int i = 0; i < 50; ++i)  map.Erase(i);
  for (int i = 100; i < 150; ++i) map.Insert(i, i);

  CHECK(map.Size() == 100);  // 50 remaining + 50 new

  for (int i = 0; i < 50; ++i)    CHECK_FALSE(map.Contains(i));
  for (int i = 50; i < 150; ++i)  CHECK(map.Contains(i));
}

// ====================================================================
//  17. Move semantics
// ====================================================================

TEST_CASE("Move construction", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> original;
  original.Insert(1, 10);
  original.Insert(2, 20);

  cse498::RobinHoodMap<int, int> moved(std::move(original));
  CHECK(moved.Size() == 2);
  CHECK(*moved.Find(1) == 10);
  CHECK(*moved.Find(2) == 20);
}

TEST_CASE("Move assignment", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> a;
  a.Insert(1, 10);

  cse498::RobinHoodMap<int, int> b;
  b.Insert(99, 99);

  b = std::move(a);
  CHECK(b.Size() == 1);
  CHECK(*b.Find(1) == 10);
  CHECK_FALSE(b.Contains(99));
}

TEST_CASE("Insert with move value semantics", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, std::string> map;
  std::string val = "moved_value";
  map.Insert(1, std::move(val));

  CHECK(map.Contains(1));
  CHECK(*map.Find(1) == "moved_value");
}

// ====================================================================
//  18. Copy semantics
// ====================================================================

TEST_CASE("Copy construction", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> original;
  original.Insert(1, 10);
  original.Insert(2, 20);
  original.Insert(3, 30);

  cse498::RobinHoodMap<int, int> copy(original);
  CHECK(copy.Size() == 3);
  CHECK(*copy.Find(1) == 10);
  CHECK(*copy.Find(2) == 20);
  CHECK(*copy.Find(3) == 30);

  // Mutating the copy does not affect the original.
  copy.Erase(1);
  CHECK(copy.Size() == 2);
  CHECK(original.Size() == 3);
  CHECK(original.Contains(1));
}

TEST_CASE("Copy assignment", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> a;
  a.Insert(1, 10);

  cse498::RobinHoodMap<int, int> b;
  b.Insert(99, 99);

  b = a;
  CHECK(b.Size() == 1);
  CHECK(*b.Find(1) == 10);
  CHECK_FALSE(b.Contains(99));

  // Original unaffected.
  CHECK(a.Size() == 1);
  CHECK(*a.Find(1) == 10);
}

// ====================================================================
//  19. Custom hash and comparator
// ====================================================================

TEST_CASE("Custom hash function", "[tools][RobinHoodMap]")
{
  // A trivial hash that maps everything to the same bucket — worst case.
  struct ConstantHash {
    size_t operator()(int) const { return 42; }
  };

  cse498::RobinHoodMap<int, int, ConstantHash> map;
  // Even with a terrible hash, correctness must be maintained.
  for (int i = 0; i < 50; ++i) {
    map.Insert(i, i * 2);
  }
  CHECK(map.Size() == 50);

  for (int i = 0; i < 50; ++i) {
    REQUIRE(map.Contains(i));
    CHECK(*map.Find(i) == i * 2);
  }

  map.Erase(25);
  CHECK_FALSE(map.Contains(25));
  CHECK(map.Size() == 49);

  // Remaining elements still intact.
  for (int i = 0; i < 50; ++i) {
    if (i == 25) continue;
    CHECK(*map.Find(i) == i * 2);
  }
}

TEST_CASE("Custom equality comparator (case-insensitive strings)", "[tools][RobinHoodMap]")
{
  // Case-insensitive hash.
  struct CIHash {
    size_t operator()(const std::string & s) const {
      size_t h = 0;
      for (char c : s) {
        h = h * 31 + static_cast<size_t>(
          static_cast<unsigned char>(std::tolower(c))
        );
      }
      return h;
    }
  };
  // Case-insensitive equality.
  struct CIEqual {
    bool operator()(const std::string & a, const std::string & b) const {
      if (a.size() != b.size()) return false;
      for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i]))) return false;
      }
      return true;
    }
  };

  cse498::RobinHoodMap<std::string, int, CIHash, CIEqual> map;
  map.Insert("Hello", 1);

  // Different case should find the same entry.
  CHECK(map.Contains("hello"));
  CHECK(map.Contains("HELLO"));
  CHECK(map.Contains("hElLo"));
  CHECK(*map.Find("HELLO") == 1);

  // Inserting with different case should update, not create new.
  map.Insert("HELLO", 2);
  CHECK(map.Size() == 1);
  CHECK(*map.Find("hello") == 2);
}

// ====================================================================
//  20. Edge cases
// ====================================================================

TEST_CASE("Single element insert, find, erase cycle", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  map.Insert(0, 0);
  CHECK(map.Size() == 1);
  CHECK(*map.Find(0) == 0);

  CHECK(map.Erase(0));
  CHECK(map.Empty());
  CHECK(map.Find(0) == nullptr);
}

TEST_CASE("Value modification through Find pointer", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  map.Insert(1, 100);

  int * val = map.Find(1);
  REQUIRE(val != nullptr);
  *val = 999;

  CHECK(*map.Find(1) == 999);
}

TEST_CASE("Value modification through operator[]", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, std::vector<int>> map;
  map[1].push_back(10);
  map[1].push_back(20);
  map[1].push_back(30);

  CHECK(map.Size() == 1);
  CHECK(map[1].size() == 3);
  CHECK(map[1][0] == 10);
  CHECK(map[1][1] == 20);
  CHECK(map[1][2] == 30);
}

TEST_CASE("Zero as a key", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, std::string> map;
  map.Insert(0, "zero");
  CHECK(map.Contains(0));
  CHECK(*map.Find(0) == "zero");
}

TEST_CASE("Negative integer keys", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map;
  map.Insert(-1, 1);
  map.Insert(-100, 100);
  map.Insert(-999, 999);

  CHECK(map.Size() == 3);
  CHECK(*map.Find(-1) == 1);
  CHECK(*map.Find(-100) == 100);
  CHECK(*map.Find(-999) == 999);
}

TEST_CASE("Empty string as key", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<std::string, int> map;
  map.Insert("", 42);
  CHECK(map.Contains(""));
  CHECK(*map.Find("") == 42);
  CHECK(map.Size() == 1);
}

TEST_CASE("Large values (vector of ints)", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, std::vector<int>> map;
  std::vector<int> big(1000, 7);
  map.Insert(1, big);

  auto * found = map.Find(1);
  REQUIRE(found != nullptr);
  CHECK(found->size() == 1000);
  CHECK((*found)[0] == 7);
  CHECK((*found)[999] == 7);
}

// ====================================================================
//  21. Rehash correctness
// ====================================================================

TEST_CASE("Automatic rehash preserves all elements", "[tools][RobinHoodMap]")
{
  // Start very small so rehash is triggered multiple times.
  cse498::RobinHoodMap<int, int> map(4);
  size_t initial_buckets = map.BucketCount();

  for (int i = 0; i < 200; ++i) {
    map.Insert(i, i);
  }

  // Bucket count must have grown.
  CHECK(map.BucketCount() > initial_buckets);
  CHECK(map.Size() == 200);

  // Every element is still retrievable with the correct value.
  for (int i = 0; i < 200; ++i) {
    REQUIRE(map.Contains(i));
    CHECK(*map.Find(i) == i);
  }
}

TEST_CASE("Iteration count matches Size after rehashes", "[tools][RobinHoodMap]")
{
  cse498::RobinHoodMap<int, int> map(2);
  for (int i = 0; i < 500; ++i) {
    map.Insert(i, i);
  }

  size_t iter_count = 0;
  for ([[maybe_unused]] auto & [k, v] : map) {
    ++iter_count;
  }
  CHECK(iter_count == map.Size());
  CHECK(iter_count == 500);
}
