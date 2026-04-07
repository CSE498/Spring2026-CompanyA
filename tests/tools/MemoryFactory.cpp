#include "../../source/tools/MemoryFactory.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/core/AgentBase.hpp"

#include <cstdint>
#include <string>
#include <vector>

// Generated some test cases with the use of an LLM to help ensure better test
// coverage Created structs via LLM to track constructor/destructor calls and
// ensure proper memory management

TEST_CASE("MemoryFactory: make/destroy via unique_ptr", "[tools][MemoryFactory]") {
    int alive = 0;
    int constructor = 0;
    int destructor = 0;

    struct Tracked {
        int &alive;
        int &constructor;
        int &destructor;

        int x{};
        std::string s{};

        Tracked(int v, std::string str, int &a, int &c, int &d) :
            alive(a), constructor(c), destructor(d), x(v), s(std::move(str)) {
            ++alive;
            ++constructor;
        }

        ~Tracked() {
            --alive;
            ++destructor;
        }

        Tracked(const Tracked &) = delete;
        Tracked &operator=(const Tracked &) = delete;
    };

    cse498::MemoryFactory<Tracked> pool(8);

    Tracked *raw = nullptr;
    {
        auto p = pool.Make(7, "hi", alive, constructor, destructor);
        raw = p.get();

        CHECK(raw != nullptr);
        CHECK(raw->x == 7);
        CHECK(raw->s == "hi");
        CHECK(alive == 1);
        CHECK(constructor == 1);
        CHECK(destructor == 0);
    }

    CHECK(alive == 0);
    CHECK(constructor == 1);
    CHECK(destructor == 1);
}

TEST_CASE("MemoryFactory: destroy(nullptr) is safe", "[tools][MemoryFactory]") {
    cse498::MemoryFactory<int> pool(4);
    pool.Destroy(nullptr);
}

TEST_CASE("MemoryFactory: reuses freed slot", "[tools][MemoryFactory]") {
    struct P {
        int v;
        explicit P(int x) : v(x) {}
    };

    cse498::MemoryFactory<P> pool(2);

    P *first = nullptr;
    {
        auto a = pool.Make(1);
        first = a.get();
    }

    P *second = nullptr;
    {
        auto b = pool.Make(2);
        second = b.get();
    }

    CHECK(second == first);
}

TEST_CASE("MemoryFactory: grows when pool is full", "[tools][MemoryFactory]") {
    struct P {
        int v;
        explicit P(int x) : v(x) {}
    };

    cse498::MemoryFactory<P> pool(2);

    auto p1 = pool.Make(1);
    auto p2 = pool.Make(2);

    CHECK(p1.get() != nullptr);
    CHECK(p2.get() != nullptr);

    // Forces allocateNewBlock()
    auto p3 = pool.Make(3);
    CHECK(p3.get() != nullptr);
}

TEST_CASE("MemoryFactory: stress test", "[tools][MemoryFactory]") {
    struct P {
        int v;
        explicit P(int x) : v(x) {}
    };

    cse498::MemoryFactory<P> pool(64);

    std::vector<decltype(pool.Make(0))> ptrs;
    ptrs.reserve(10000);

    for (int i = 0; i < 10000; ++i) {
        ptrs.push_back(pool.Make(i));
    }

    ptrs.clear(); // automatic destruction
}

//
// Agent Integration Tests
//

TEST_CASE("MemoryFactory with Agents: can construct/destroy PacingAgent with "
          "MazeWorld",
          "[tools][MemoryFactory][agents]") {
    cse498::MazeWorld world;
    cse498::MemoryFactory<cse498::PacingAgent> pool(4);

    auto a = pool.Make(0, std::string("pace"), world);
    REQUIRE(a != nullptr);

    CHECK(a->IsAgent());
    CHECK(a->GetSymbol() == '*');
    a->SetSymbol('P');
    CHECK(a->GetSymbol() == 'P');
}

TEST_CASE("MemoryFactory with Agents: slot reuse works with PacingAgent", "[tools][MemoryFactory][agents]") {
    cse498::MazeWorld world;
    cse498::MemoryFactory<cse498::PacingAgent> pool(1);

    cse498::PacingAgent *first = nullptr;
    {
        auto a1 = pool.Make(0, std::string("a1"), world);
        first = a1.get();
    }

    cse498::PacingAgent *second = nullptr;
    {
        auto a2 = pool.Make(1, std::string("a2"), world);
        second = a2.get();
    }

    CHECK(second == first);
}

TEST_CASE("MemoryFactory and Agents: make() returns memory to pool for PacingAgent", "[tools][MemoryFactory][agents]") {
    cse498::MazeWorld world;
    cse498::MemoryFactory<cse498::PacingAgent> pool(1);

    cse498::PacingAgent *raw = nullptr;
    {
        auto up = pool.Make(0, std::string("pace"), world);
        raw = up.get();
        REQUIRE(raw != nullptr);
        raw->SetSymbol('X');
        CHECK(raw->GetSymbol() == 'X');
    }

    auto again = pool.Make(1, std::string("again"), world);
    CHECK(again.get() == raw);
}
