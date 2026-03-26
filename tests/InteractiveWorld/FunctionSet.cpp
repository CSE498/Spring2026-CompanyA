#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/InteractiveWorld/FunctionSet.hpp"

#include <vector>

using namespace cse498;

TEST_CASE("FunctionSet starts empty")
{
  FunctionSet<void()> fs;

  CHECK(fs.Size() == 0);
  CHECK(fs.Empty() == true);
}

TEST_CASE("AddFunction increases size")
{
  FunctionSet<void()> fs;

  auto id0 = fs.AddFunction([] {});
  auto id1 = fs.AddFunction([] {});

  CHECK(id1 != id0);
  CHECK(fs.Size() == 2);
  CHECK(fs.Empty() == false);
}

TEST_CASE("RemoveFunction removes the correct function")
{
  FunctionSet<void()> fs;

  auto id0 = fs.AddFunction([] {});
  auto id1 = fs.AddFunction([] {});

  CHECK(fs.RemoveFunction(id0) == true);
  CHECK(fs.Size() == 1);

  CHECK(fs.RemoveFunction(id0) == false);
  CHECK(fs.RemoveFunction(id1) == true);
  CHECK(fs.Size() == 0);
  CHECK(fs.Empty() == true);
}

TEST_CASE("Clear removes all functions")
{
  FunctionSet<void()> fs;

  fs.AddFunction([] {});
  fs.AddFunction([] {});

  CHECK(fs.Size() == 2);

  fs.Clear();

  CHECK(fs.Size() == 0);
  CHECK(fs.Empty() == true);
}

TEST_CASE("CallAll calls functions in insertion order")
{
  FunctionSet<void(int)> fs;
  std::vector<int> out;

  fs.AddFunction([&](int x) { out.push_back(x); });
  fs.AddFunction([&](int x) { out.push_back(x + 1); });
  fs.AddFunction([&](int x) { out.push_back(x + 2); });

  fs.CallAll(10);

  REQUIRE(out.size() == 3);
  CHECK(out[0] == 10);
  CHECK(out[1] == 11);
  CHECK(out[2] == 12);
}

TEST_CASE("Removed functions are not called")
{
  FunctionSet<void()> fs;
  int called = 0;

  auto id0 = fs.AddFunction([&] { ++called; });
  auto id1 = fs.AddFunction([&] { ++called; });

  CHECK(fs.RemoveFunction(id1) == true);

  fs.CallAll();

  CHECK(called == 1);

  CHECK(id0 != id1);
}

TEST_CASE("Function IDs keep increasing after Clear")
{
  FunctionSet<void()> fs;

  auto id0 = fs.AddFunction([] {});
  fs.Clear();
  auto id1 = fs.AddFunction([] {});

  CHECK(id1 > id0);
}

TEST_CASE("IDs view returns stored IDs in order")
{
  FunctionSet<void()> fs;

  auto id0 = fs.AddFunction([] {});
  auto id1 = fs.AddFunction([] {});
  auto id2 = fs.AddFunction([] {});

  std::vector<FunctionSet<void()>::FunctionID> ids;
  for (auto id : fs.IDs()) {
    ids.push_back(id);
  }

  REQUIRE(ids.size() == 3);
  CHECK(ids[0] == id0);
  CHECK(ids[1] == id1);
  CHECK(ids[2] == id2);
}

TEST_CASE("Entries view exposes ids in insertion order")
{
  FunctionSet<void()> fs;

  auto id0 = fs.AddFunction([] {});
  auto id1 = fs.AddFunction([] {});

  std::vector<FunctionSet<void()>::FunctionID> ids;
  for (const auto& entry : fs.Entries()) {
    ids.push_back(entry.id);
  }

  REQUIRE(ids.size() == 2);
  CHECK(ids[0] == id0);
  CHECK(ids[1] == id1);
}