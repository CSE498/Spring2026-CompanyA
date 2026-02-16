
#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/MemoryFactory.hpp"

#include "../../source/core/AgentBase.hpp"
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/Agents/PacingAgent.hpp"

#include <cstdint>
#include <string>
#include <vector>

// Generated some test cases with the use of an LLM to help ensure better test coverage
// Created structs via LLM to track constructor/destructor calls and ensure proper memory management

TEST_CASE("MemoryFactory: basic create/destroy", "[tools][MemoryFactory]")
{
  int alive = 0;
  int constructor  = 0;
  int destructor  = 0;

  struct Tracked {
    int& alive;
    int& constructor;
    int& destructor;

    int x{};
    std::string s{};

    Tracked(int v, std::string str, int& a, int& c, int& d)
      : alive(a), constructor(c), destructor(d), x(v), s(std::move(str))
    {
      ++alive;
      ++constructor;
    }

    ~Tracked() {
      --alive;
      ++destructor;
    }

    Tracked(const Tracked&) = delete;
    Tracked& operator=(const Tracked&) = delete;
  };

  cse498::MemoryFactory<Tracked> pool(8);

  Tracked* p = pool.create(7, "hi", alive, constructor, destructor);
  CHECK(p != nullptr);
  CHECK(p->x == 7);
  CHECK(p->s == "hi");
  CHECK(alive == 1);
  CHECK(constructor == 1);
  CHECK(destructor == 0);

  pool.destroy(p);
  CHECK(alive == 0);
  CHECK(constructor == 1);
  CHECK(destructor == 1);
}

TEST_CASE("MemoryFactory: destroy(nullptr) is safe", "[tools][MemoryFactory]")
{
  cse498::MemoryFactory<int> pool(4);
  pool.destroy(nullptr);
}

TEST_CASE("MemoryFactory: reuses freed slot", "[tools][MemoryFactory]")
{
  struct P { int v; explicit P(int x) : v(x) {} };

  cse498::MemoryFactory<P> pool(2);

  P* a = pool.create(1);
  pool.destroy(a);

  P* b = pool.create(2);

  // This implementation uses a LIFO free list, so the most recently freed
  // slot should be reused first
  CHECK(b == a);

  pool.destroy(b);
}

TEST_CASE("MemoryFactory: grows when pool is full", "[tools][MemoryFactory]")
{
  struct P { int v; explicit P(int x) : v(x) {} };

  cse498::MemoryFactory<P> pool(2);

  P* p1 = pool.create(1);
  P* p2 = pool.create(2);
  CHECK(p1 != nullptr);
  CHECK(p2 != nullptr);

  // Should force allocateNewBlock()
  P* p3 = pool.create(3);
  CHECK(p3 != nullptr);

  pool.destroy(p1);
  pool.destroy(p2);
  pool.destroy(p3);
}

TEST_CASE("MemoryFactory: make() returns unique_ptr that returns memory to pool", "[tools][MemoryFactory]")
{
  int alive = 0;

  struct Tracked {
    int& alive;
    int x{};
    Tracked(int v, int& a) : alive(a), x(v) { ++alive; }
    ~Tracked() { --alive; }

    Tracked(const Tracked&) = delete;
    Tracked& operator=(const Tracked&) = delete;
  };

  cse498::MemoryFactory<Tracked> pool(4);

  Tracked* raw = nullptr;
  {
    auto up = pool.make(10, alive);
    raw = up.get();

    CHECK(raw != nullptr);
    CHECK(raw->x == 10);
    CHECK(alive == 1);
  } // unique_ptr destroyed here

  CHECK(alive == 0);

  // After destruction, the next allocation should reuse the freed slot.
  Tracked* again = pool.create(11, alive);
  CHECK(again == raw);
  pool.destroy(again);
  CHECK(alive == 0);
}

TEST_CASE("MemoryFactory: stress test", "[tools][MemoryFactory]")
{
  struct P { int v; explicit P(int x) : v(x) {} };

  cse498::MemoryFactory<P> pool(64);

  std::vector<P*> ptrs;
  ptrs.reserve(10000);

  for (int i = 0; i < 10000; ++i) {
    ptrs.push_back(pool.create(i));
  }

  for (auto* p : ptrs) {
    pool.destroy(p);
  }
}

TEST_CASE("MemoryFactory with Agents: can construct/destroy PacingAgent with MazeWorld",
          "[tools][MemoryFactory][agents]")
{
  cse498::MazeWorld world;
  cse498::MemoryFactory<cse498::PacingAgent> pool(4);

  auto* a = pool.create(0, std::string("pace"), world);
  REQUIRE(a != nullptr);

  CHECK(a->IsAgent());
  CHECK(a->GetSymbol() == '*');
  a->SetSymbol('P');
  CHECK(a->GetSymbol() == 'P');

  pool.destroy(a);
}

TEST_CASE("MemoryFactory with Agents: slot reuse works with PacingAgent",
          "[tools][MemoryFactory][agents]")
{
  cse498::MazeWorld world;
  cse498::MemoryFactory<cse498::PacingAgent> pool(1);

  auto* a1 = pool.create(0, std::string("a1"), world);
  pool.destroy(a1);

  auto* a2 = pool.create(1, std::string("a2"), world);

  CHECK(a2 == a1);

  pool.destroy(a2);
}

TEST_CASE("MemoryFactory and Agents: make() returns memory to pool for PacingAgent",
          "[tools][MemoryFactory][agents]")
{
  cse498::MazeWorld world;
  cse498::MemoryFactory<cse498::PacingAgent> pool(1);

  cse498::PacingAgent* raw = nullptr;
  {
    auto up = pool.make(0, std::string("pace"), world);
    raw = up.get();
    REQUIRE(raw != nullptr);
    raw->SetSymbol('X');
    CHECK(raw->GetSymbol() == 'X');
  } 

  auto* again = pool.create(1, std::string("again"), world);
  CHECK(again == raw);

  pool.destroy(again);
}