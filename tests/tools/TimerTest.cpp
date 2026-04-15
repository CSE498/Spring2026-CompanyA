#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/Timer.hpp"

TEST_CASE("Timer Constructor", "[Timer]") {
  // Test that single parameter constructor defaults the Timer to running.
  cse498::Timer Timer("Test1");
  REQUIRE(Timer.isRunning());
  Timer.advanceTime(1.0);
  REQUIRE(Timer.elapsed() > 0);

  // Test that adding true in the constructor functions properly and starts the
  // Timer running.
  cse498::Timer Timer2("Test2", true);
  REQUIRE(Timer2.isRunning());
  Timer2.advanceTime(1.0);
  REQUIRE(Timer2.elapsed() > 0);

  // Test that adding false in the constructor starts the Timer paused
  // and that the elapsed time defualts to 0.
  cse498::Timer Timer3("Test3", false);
  REQUIRE(!Timer3.isRunning());
  Timer3.advanceTime(1.0);
  REQUIRE(Timer3.elapsed() == 0.0);
}

TEST_CASE("Timer Start Method", "[Timer]") {
  // Create a Timer stopped at creation and double check that the state is as
  // intended.
  cse498::Timer Timer("Test1", false);
  REQUIRE(!Timer.isRunning());
  Timer.advanceTime(1.0);
  REQUIRE(Timer.elapsed() == 0.0);

  // Call start on the Timer and make sure that it is running and that time is
  // elapsing.
  Timer.start();
  REQUIRE(Timer.isRunning());
  Timer.advanceTime(1.0);
  REQUIRE(Timer.elapsed() > 0);

  // Make sure that calling start on a running Timer doesn't change the state.
  Timer.start();
  REQUIRE(Timer.isRunning());
}

TEST_CASE("Timer Stop Method", "[Timer]") {
  // Create a Timer stopped at creation and double check that the state is as
  // intended.
  cse498::Timer Timer("Test1", false);
  REQUIRE(!Timer.isRunning());
  REQUIRE(Timer.elapsed() == 0.0);

  // Call start on the Timer and make sure that it is running and that time is
  // elapsing.
  Timer.start();
  REQUIRE(Timer.isRunning());
  Timer.advanceTime(1.0);
  REQUIRE(Timer.elapsed() > 0);

  // Call stop on the Timer and make sure that it is no longer running.
  Timer.stop();
  double elapsed1 = Timer.elapsed();
  REQUIRE(!Timer.isRunning());

  // Make sure that calling stop on a paused Timer doesn't change the state or
  // increase the elapsed time.
  Timer.stop();
  double elapsed2 = Timer.elapsed();
  REQUIRE(!Timer.isRunning());
  REQUIRE(elapsed1 == elapsed2);
}

TEST_CASE("Timer Elapsed Method", "[Timer]") {
  // Create a Timer stopped at creation and double check that the state is as
  // intended.
  cse498::Timer Timer("Test1", false);
  REQUIRE(!Timer.isRunning());
  REQUIRE(Timer.elapsed() == 0.0);

  // Call start on the Timer and mock a 2 second passage of time,
  // making sure that the Timer correctly handles it.
  Timer.start();
  REQUIRE(Timer.isRunning());
  Timer.advanceTime(2.0);
  Timer.stop();
  REQUIRE(Timer.elapsed() == Approx(2.0).margin(0.01));

  // Advance time while the Timer is paused and make sure it doesn't increase
  // the elapsed time.
  Timer.advanceTime(1.0);
  REQUIRE(Timer.elapsed() == Approx(2.0).margin(0.01));

  // Start the Timer back up and advance time by some less conformative
  // intervals.
  Timer.start();
  Timer.advanceTime(1.64);
  REQUIRE(Timer.elapsed() == Approx(3.64).margin(0.01));
  Timer.advanceTime(0.93);
  REQUIRE(Timer.elapsed() == Approx(4.57).margin(0.01));
}

TEST_CASE("Timer Reset Method", "[Timer]") {
  // Start a Timer, advance some time, stop the Timer,
  // then call reset and make sure it's elapsed time is reset and state is
  // unchanged.
  cse498::Timer Timer("Test1");
  REQUIRE(Timer.isRunning());
  Timer.advanceTime(4.69);
  REQUIRE(Timer.elapsed() == Approx(4.69).margin(0.01));
  Timer.stop();
  REQUIRE(!Timer.isRunning());
  Timer.reset();
  REQUIRE(!Timer.isRunning());
  REQUIRE(Timer.elapsed() == 0.0);

  // Test that reset still functions properly when called on a running Timer as
  // opposed to a stopped one.
  cse498::Timer Timer2("Test2");
  REQUIRE(Timer2.isRunning());
  Timer2.advanceTime(2.64);
  REQUIRE(Timer2.elapsed() == Approx(2.64).margin(0.01));
  Timer2.reset();
  REQUIRE(!Timer2.isRunning());
  REQUIRE(Timer2.elapsed() == 0.0);

  // Test that calling reset back to back doesn't mess anything up.
  Timer2.reset();
  REQUIRE(!Timer2.isRunning());
  REQUIRE(Timer2.elapsed() == 0.0);
}

TEST_CASE("Timer Restart Method", "[Timer]") {
  // Start a Timer, advance some time, stop the Timer,
  // then call restart and make sure it's elapsed time is reset but it is
  // started back up again.
  cse498::Timer Timer("Test1");
  REQUIRE(Timer.isRunning());
  Timer.advanceTime(7.22);
  REQUIRE(Timer.elapsed() == Approx(7.22).margin(0.01));
  Timer.stop();
  REQUIRE(!Timer.isRunning());
  Timer.restart();
  REQUIRE(Timer.isRunning());
  REQUIRE(Timer.elapsed() == Approx(0.00).margin(0.01));

  // Test that restart still functions properly when called on a running Timer
  // as opposed to a stopped one.
  cse498::Timer Timer2("Test2");
  REQUIRE(Timer2.isRunning());
  Timer2.advanceTime(8.15);
  REQUIRE(Timer2.elapsed() == Approx(8.15).margin(0.01));
  Timer2.restart();
  REQUIRE(Timer2.isRunning());
  REQUIRE(Timer2.elapsed() == Approx(0.00).margin(0.01));

  // Test that calling it again shortly after the first execution still has the
  // same outcome.
  Timer2.restart();
  REQUIRE(Timer2.isRunning());
  REQUIRE(Timer2.elapsed() == Approx(0.00).margin(0.01));
}

TEST_CASE("Timer Lap Method", "[Timer]") {
  // Start a Timer and make sure that the first lap time is updating properly
  // alongside the Timer.
  cse498::Timer Timer("Test1");
  REQUIRE(Timer.isRunning());
  Timer.stop();
  REQUIRE(Timer.getLaps().at(0) == Approx(Timer.elapsed()).margin(0.01));

  // Start the Timer back up, advance the time a little bit, lap it,
  // make sure that another lap was added to the Timer, and that the first one
  // properly recorded time.
  Timer.start();
  Timer.advanceTime(0.45);
  Timer.lap();
  REQUIRE(Timer.getLaps().size() == 2);
  REQUIRE(Timer.getLaps().at(0) == Approx(0.45).margin(0.01));

  // Advance some more time and make sure that lapping more than once is handled
  // properly without messing with previous values.
  Timer.advanceTime(0.63);
  Timer.lap();
  REQUIRE(Timer.getLaps().size() == 3);
  REQUIRE(Timer.getLaps().at(0) == Approx(0.45).margin(0.01));
  REQUIRE(Timer.getLaps().at(1) == Approx(0.63).margin(0.01));
}

TEST_CASE("Timer String Method", "[Timer]") {
  cse498::Timer timer("Test", false);
  REQUIRE(timer.toString(false) == "Test [STOPPED]: 00:00.000");

  timer.start();
  timer.advanceTime(5.27);
  REQUIRE(timer.toString(false) == "Test [RUNNING]: 00:05.270");
  REQUIRE(timer.toString(true) ==
          "Test [RUNNING]: 00:05.270\n  Lap 1: 00:05.270");

  timer.advanceTime(75.0);
  timer.stop();
  REQUIRE(timer.toString(false) == "Test [STOPPED]: 01:20.270");

  timer.lap();
  REQUIRE(timer.toString(false) == "Test [STOPPED]: 01:20.270");

  timer.start();
  timer.lap();
  REQUIRE(timer.toString(true) ==
          "Test [RUNNING]: 01:20.270\n  Lap 1: 01:20.270\n  Lap 2: 00:00.000");

  timer.advanceTime(3.82);
  REQUIRE(timer.toString(true) ==
          "Test [RUNNING]: 01:24.090\n  Lap 1: 01:20.270\n  Lap 2: 00:03.820");
}