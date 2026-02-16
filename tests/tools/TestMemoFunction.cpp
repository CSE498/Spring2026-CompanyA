/**
 * @file TestMemoFunction.cpp
 * @author Henry Finfrock
 *
 * Catch2 unit tests for MemoFunction.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/MemoFunction.hpp"

#include <string>
#include <vector>
#include <chrono>
#include <iostream>

using namespace cse498;
using namespace std::chrono;
using std::string, std::vector;

int multby3(int x) { return x * 3; }

TEST_CASE("MemoFunction: Basic function and cache testing", "[MemoFunction]")
{
    MemoFunction<int, int> mf1(multby3);
    REQUIRE(mf1.CacheSize() == 0);
    REQUIRE(mf1(7) == 21);
    REQUIRE(mf1.CacheSize() == 1);
    REQUIRE(mf1(11) == 33);
    REQUIRE(mf1.CacheSize() == 2);
    REQUIRE(mf1(7) == 21);
    REQUIRE(mf1.CacheSize() == 2);
    mf1.CacheClear();
    REQUIRE(mf1.CacheSize() == 0);
    REQUIRE(mf1(3) == 9);
    REQUIRE(mf1.CacheSize() == 1);
}

class testclass {
    public:
      testclass() {};
      string concat(string a, string b) { return a + b; }
      bool even(int x) { return x % 2 == 0; }
      double add(double a, double b, int c) { return a + b + c; }
};

// Can apparently pass class functions directly into memofunction with template stuff but not
// sure if it's really necessary, hopefully using lambda for them isn't that unintuitive
TEST_CASE("MemoFunction: Class member functions w/ multiple inputs and types", "[MemoFunction]")
{
    testclass t1;
    auto l1 = [&](string a, string b) { return t1.concat(a, b); };
    MemoFunction<string, string, string> mf2(l1);
    REQUIRE(mf2("abc", "def") == "abcdef");
    REQUIRE(mf2.CacheSize() == 1);

    auto l2 = [&](int x) { return t1.even(x); };
    MemoFunction<bool, int> mf3(l2);
    REQUIRE(mf3(2));
    REQUIRE(!mf3(3));
    REQUIRE(mf3.CacheSize() == 2);

    auto l3 = [&](double a, double b, int c) { return t1.add(a, b, c); };
    MemoFunction<double, double, double, int> mf4(l3);
    REQUIRE(mf4.CacheSize() == 0);
    REQUIRE(mf4(2.2, 3.6, 1) == Approx(6.8));
    REQUIRE(mf4.CacheSize() == 1);
    REQUIRE(mf4(3.6, 2.2, 1) == Approx(6.8));
    REQUIRE(mf4.CacheSize() == 2);
    mf4.CacheClear();
    REQUIRE(mf4.CacheSize() == 0);
}

vector<char> hello() { return {'h', 'e', 'l', 'l', 'o'}; }

TEST_CASE("MemoFunction: No argument", "[Memofunction]")
{
    MemoFunction<vector<char>> mf5(hello);
    auto v = mf5();
    REQUIRE(string(v.begin(), v.end()) == "hello");
    REQUIRE(mf5.CacheSize() == 1);
}

int exception() { throw std::logic_error("test"); return 1; }

TEST_CASE("MemoFunction: Exceptions", "[MemoFunction]")
{
    MemoFunction<int> mf6(exception);
    REQUIRE_THROWS_AS(mf6(), std::logic_error);
    REQUIRE(mf6.CacheSize() == 0);

    std::function<int()> f;
    MemoFunction<int> mf7(f);
    REQUIRE_THROWS_AS(mf7(), std::bad_function_call);
    REQUIRE(mf7.CacheSize() == 0);
}

int fib(int x) {
  if (x < 2) {
    return x;
  } else {
    return fib(x - 2) + fib(x - 1);
  }
}

TEST_CASE("MemoFunction: Performance", "[MemoFunction]")
{
    MemoFunction<int, int> mf8(fib);

    auto start1 = high_resolution_clock::now();
    mf8(30);
    auto end1 = high_resolution_clock::now();
    REQUIRE(mf8.CacheSize() == 1);

    auto start2 = high_resolution_clock::now();
    mf8(30);
    auto end2 = high_resolution_clock::now();
    REQUIRE(mf8.CacheSize() == 1);

    auto time1 = duration_cast<microseconds>(end1 - start1).count();
    auto time2 = duration_cast<microseconds>(end2 - start2).count();
    REQUIRE(time1 > time2 * 100);

    //std::cout << "Time 1:" << time1 << std::endl;
    //std::cout << "Time 2:" << time2 << std::endl;
}

TEST_CASE("MemoFunction: Cache limit", "[MemoFunction]")
{
    MemoFunction<int, int> mf9(multby3);
    mf9.SetLimit(2);
    mf9(1); // cache = {1}
    REQUIRE(mf9.CacheSize() == 1);
    mf9(2); // cache = {1, 2}
    REQUIRE(mf9.CacheSize() == 2);
    mf9(3); // cache = {2, 3}, oldest deleted
    REQUIRE(!mf9.IsCached(1));
    REQUIRE(mf9.CacheSize() == 2);
    mf9.SetLimit(3);
    mf9(2); // cache = {2, 3} still
    REQUIRE(mf9.CacheSize() == 2);
    mf9(1); // cache = {2, 3, 1}
    REQUIRE(mf9.CacheSize() == 3);
    mf9.CacheClear(); // cache = {}
    REQUIRE(mf9.CacheSize() == 0);
    mf9(1); // cache = {1}
    REQUIRE(mf9.CacheSize() == 1);
}