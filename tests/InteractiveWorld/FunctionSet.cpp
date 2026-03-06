#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/InteractiveWorld/FunctionSet.hpp"

#include <stdexcept>
#include <vector>
#include <string_view>

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
  CHECK(fs.LastCallSuccessCount() == 2);
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
  CHECK(fs.LastCallSuccessCount() == 2);
}

TEST_CASE("Test to ensure Clear removes all functions but does not reuse IDs")
{
  FunctionSet<void()> fs;

  auto id0 = fs.AddFunction([] {});
  fs.Clear();
  CHECK(fs.Size() == 0);

  auto id1 = fs.AddFunction([] {});
  CHECK(id1 != id0);
  CHECK(id1 > id0);  
}

TEST_CASE("Removed functions are not called by CallAll")
{
  FunctionSet<void()> fs;
  int called = 0;

  auto id0 = fs.AddFunction([&] { ++called; });
  (void)id0;
  auto id1 = fs.AddFunction([&] { ++called; });

  CHECK(fs.RemoveFunction(id1) == true);
  fs.CallAll();

  CHECK(called == 1);
  CHECK(fs.LastCallFailureCount() == 0);
}

TEST_CASE("CallAll invokes on_error for throwing functions")
{
  FunctionSet<void()> fs;

  auto id_ok = fs.AddFunction([] {});
  (void)id_ok;

  auto id_std = fs.AddFunction([] { throw std::runtime_error("fail"); });
  auto id_nonstd = fs.AddFunction([] { throw 123; });

  int calls = 0;
  bool saw_std = false;
  bool saw_nonstd = false;

  fs.CallAll([&](auto id, std::string_view msg) {
    ++calls;

    if (id == id_std) {
      CHECK(msg == "fail");
      saw_std = true;
    } else if (id == id_nonstd) {
      CHECK(msg == "Function threw non-std exception");
      saw_nonstd = true;
    } else {
      FAIL("on_error called with unexpected FunctionID");
    }
  });

  CHECK(fs.LastCallFailureCount() == 2);
  CHECK(fs.LastCallSuccessCount() == 1);

  CHECK(calls == 2);
  CHECK(saw_std);
  CHECK(saw_nonstd);
}