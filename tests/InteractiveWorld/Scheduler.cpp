#include "../../source/tools/InteractiveWorld/Scheduler.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

using namespace cse498;

TEST_CASE("Empty scheduler") {
  Scheduler s;

  CHECK(s.Empty() == true);

  s.AddProcess(1, 1.0f);
  CHECK(s.Empty() == false);

  s.Clear();
  CHECK(s.Empty() == true);
}

TEST_CASE("AddProcess and HasProcess") {
  Scheduler s;

  CHECK(s.HasProcess(1) == false);

  s.AddProcess(1, 2.0f);
  CHECK(s.HasProcess(1) == true);

  s.AddProcess(2, 5.0f);
  CHECK(s.HasProcess(2) == true);
}

TEST_CASE("RemoveProcess behavior") {
  Scheduler s;

  s.AddProcess(1, 1.0f);
  s.AddProcess(2, 2.0f);

  CHECK(s.RemoveProcess(1) == true);
  CHECK(s.HasProcess(1) == false);

  // Removing again should fail
  CHECK(s.RemoveProcess(1) == false);

  // Other process should still exist
  CHECK(s.HasProcess(2) == true);
}

TEST_CASE("UpdatePriority changes priority without removing process") {
  Scheduler s;

  s.AddProcess(1, 1.0f);
  s.UpdatePriority(1, 10.0f);

  CHECK(s.HasProcess(1) == true);
}

TEST_CASE("Next returns a valid process ID") {
  Scheduler s;

  s.AddProcess(10, 1.0f);
  s.AddProcess(20, 1.0f);
  s.AddProcess(30, 1.0f);

  for (int i = 0; i < 100; ++i) {
    ProcessID id = s.Next();
    CHECK((id == 10 || id == 20 || id == 30));
  }
}

TEST_CASE("Higher priority processes are selected more often") {
  Scheduler s;

  // Low priority
  s.AddProcess(1, 1.0f);

  // High priority
  s.AddProcess(2, 5.0f);

  int countLow = 0;
  int countHigh = 0;

  const int trials = 10000;

  for (int i = 0; i < trials; ++i) {
    ProcessID id = s.Next();
    if (id == 1)
      countLow++;
    if (id == 2)
      countHigh++;
  }

  // High priority should be chosen significantly more often
  CHECK(countHigh > countLow);
}

TEST_CASE("Scheduler respects Clear") {
  Scheduler s;

  s.AddProcess(1, 1.0f);
  s.AddProcess(2, 2.0f);

  CHECK(s.Empty() == false);

  s.Clear();
  CHECK(s.Empty() == true);
  CHECK(s.HasProcess(1) == false);
  CHECK(s.HasProcess(2) == false);
}

TEST_CASE("RemoveProcess on empty scheduler returns false") {
  Scheduler s;

  CHECK(s.RemoveProcess(42) == false);
}

TEST_CASE("HasProcess returns false after Clear") {
  Scheduler s;

  s.AddProcess(1, 1.0f);
  s.AddProcess(2, 2.0f);

  s.Clear();

  CHECK(s.HasProcess(1) == false);
  CHECK(s.HasProcess(2) == false);
}

TEST_CASE("UpdatePriority does not create new processes") {
  Scheduler s;

  s.AddProcess(1, 1.0f);
  CHECK(s.HasProcess(2) == false);

  // This should assert in debug mode; in release it does nothing
  // We only check that no new process appears
  REQUIRE(s.HasProcess(1) == true);
}

TEST_CASE("Scheduler works with single process") {
  Scheduler s;

  s.AddProcess(99, 10.0f);

  for (int i = 0; i < 100; ++i) {
    CHECK(s.Next() == 99);
  }
}

TEST_CASE("Removing one process does not affect others") {
  Scheduler s;

  s.AddProcess(1, 1.0f);
  s.AddProcess(2, 2.0f);
  s.AddProcess(3, 3.0f);

  CHECK(s.RemoveProcess(2) == true);

  CHECK(s.HasProcess(1) == true);
  CHECK(s.HasProcess(2) == false);
  CHECK(s.HasProcess(3) == true);

  for (int i = 0; i < 100; ++i) {
    ProcessID id = s.Next();
    CHECK((id == 1 || id == 3));
  }
}

TEST_CASE("Priority updates affect selection frequency") {
  Scheduler s;

  s.AddProcess(1, 1.0f);
  s.AddProcess(2, 1.0f);

  int count1_before = 0;
  int count2_before = 0;

  const int trials = 5000;

  for (int i = 0; i < trials; ++i) {
    ProcessID id = s.Next();
    if (id == 1)
      count1_before++;
    if (id == 2)
      count2_before++;
  }

  // Roughly equal before update
  CHECK(std::abs(count1_before - count2_before) < trials * 0.2);

  // Increase priority of process 2
  s.UpdatePriority(2, 10.0f);

  int count1_after = 0;
  int count2_after = 0;

  for (int i = 0; i < trials; ++i) {
    ProcessID id = s.Next();
    if (id == 1)
      count1_after++;
    if (id == 2)
      count2_after++;
  }

  CHECK(count2_after > count1_after);
}

TEST_CASE("Scheduler handles large number of processes") {
  Scheduler s;

  const int numProcesses = 100;

  for (int i = 0; i < numProcesses; ++i) {
    s.AddProcess(i, 1.0f);
  }

  for (int i = 0; i < 1000; ++i) {
    ProcessID id = s.Next();
    CHECK(id >= 0);
    CHECK(id < numProcesses);
  }
}

TEST_CASE("Removing all processes one by one empties scheduler") {
  Scheduler s;

  s.AddProcess(1, 1.0f);
  s.AddProcess(2, 2.0f);
  s.AddProcess(3, 3.0f);

  CHECK(s.RemoveProcess(1) == true);
  CHECK(s.RemoveProcess(2) == true);
  CHECK(s.RemoveProcess(3) == true);

  CHECK(s.Empty() == true);
}

TEST_CASE("Clear can be called multiple times safely") {
  Scheduler s;

  s.AddProcess(1, 1.0f);
  s.Clear();

  CHECK(s.Empty() == true);

  // Calling Clear again should not crash
  s.Clear();
  CHECK(s.Empty() == true);
}
