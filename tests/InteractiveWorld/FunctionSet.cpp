#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../group_specific_content/Group-14/Group14InitialClasses/FunctionSet.hpp"

#include <stdexcept>
#include <vector>

using namespace cse498;

TEST_CASE("Test add, remove, size functions")
{
  FunctionSet<void()> fs;

  CHECK(fs.Size() == 0);

  auto id0 = fs.AddFunction([] {});
  auto id1 = fs.AddFunction([] {});

  CHECK(fs.Size() == 2);

  CHECK(fs.RemoveFunction(id0) == true);
  CHECK(fs.Size() == 1);

  CHECK(fs.RemoveFunction(999999) == false);
  CHECK(fs.Size() == 1);

  CHECK(fs.RemoveFunction(id1) == true);
  CHECK(fs.Size() == 0);
}

TEST_CASE("Test CallAll to properly call all functions in order")
{
  FunctionSet<void(int)> fs;
  std::vector<int> out;

  fs.AddFunction([&](int x) { out.push_back(x); });
  fs.AddFunction([&](int x) { out.push_back(x + 1); });

  fs.CallAll(10);

  REQUIRE(out.size() == 2);
  CHECK(out[0] == 10);
  CHECK(out[1] == 11);

  CHECK(fs.LastCallFailureCount() == 0);
}

TEST_CASE("Test CallAll to continue after throw and count failures")
{
  FunctionSet<void()> fs;
  int called = 0;

  fs.AddFunction([&] { ++called; });
  fs.AddFunction([&] { throw std::runtime_error("fail"); });
  fs.AddFunction([&] { ++called; });

  fs.CallAll();

  CHECK(called == 2);
  CHECK(fs.LastCallFailureCount() == 1);
}

TEST_CASE("Test to ensure Clear removes all functions and reset IDs")
{
  FunctionSet<void()> fs;

  auto id0 = fs.AddFunction([] {});
  CHECK(id0 == 0);

  fs.Clear();
  CHECK(fs.Size() == 0);

  auto id1 = fs.AddFunction([] {});
  CHECK(id1 == 0);
}