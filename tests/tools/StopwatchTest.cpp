#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/Stopwatch.hpp"

TEST_CASE("Stopwatch Constructor", "[Stopwatch]")
{
    // Test that single parameter constructor defaults the Stopwatch to running.
    cse498::Stopwatch stopwatch("Test1");
    REQUIRE(stopwatch.isRunning());
    REQUIRE(stopwatch.elapsed() > 0);

    // Test that adding true in the constructor functions properly and starts the Stopwatch running.
    cse498::Stopwatch stopwatch2("Test2", true);
    REQUIRE(stopwatch2.isRunning());
    REQUIRE(stopwatch2.elapsed() > 0);

    // Test that adding false in the constructor starts the Stopwatch paused
    // and that the elapsed time defualts to 0.
    cse498::Stopwatch stopwatch3("Test3", false);
    REQUIRE(!stopwatch3.isRunning());
    REQUIRE(stopwatch3.elapsed() == 0.0);
}

TEST_CASE("Stopwatch Start Method", "[Stopwatch]")
{
    // Create a Stopwatch stopped at creation and double check that the state is as intended.
    cse498::Stopwatch stopwatch("Test1", false);
    REQUIRE(!stopwatch.isRunning());
    REQUIRE(stopwatch.elapsed() == 0.0);

    // Call start on the Stopwatch and make sure that it is running and that time is elapsing.
    stopwatch.start();
    REQUIRE(stopwatch.isRunning());
    REQUIRE(stopwatch.elapsed() > 0);

    // Make sure that calling start on a running Stopwatch doesn't change the state.
    stopwatch.start();
    REQUIRE(stopwatch.isRunning());
}

TEST_CASE("Stopwatch Stop Method", "[Stopwatch]")
{
    // Create a Stopwatch stopped at creation and double check that the state is as intended.
    cse498::Stopwatch stopwatch("Test1", false);
    REQUIRE(!stopwatch.isRunning());
    REQUIRE(stopwatch.elapsed() == 0.0);

    // Call start on the Stopwatch and make sure that it is running and that time is elapsing.
    stopwatch.start();
    REQUIRE(stopwatch.isRunning());
    REQUIRE(stopwatch.elapsed() > 0);

    // Call stop on the Stopwatch and make sure that it is no longer running.
    stopwatch.stop();
    double elapsed1 = stopwatch.elapsed();
    REQUIRE(!stopwatch.isRunning());

    // Make sure that calling stop on a paused Stopwatch doesn't change the state or increase the elapsed time.
    stopwatch.stop();
    double elapsed2 = stopwatch.elapsed();
    REQUIRE(!stopwatch.isRunning());
    REQUIRE(elapsed1 == elapsed2);
}

TEST_CASE("Stopwatch Elapsed Method", "[Stopwatch]")
{
    // Create a Stopwatch stopped at creation and double check that the state is as intended.
    cse498::Stopwatch stopwatch("Test1", false);
    REQUIRE(!stopwatch.isRunning());
    REQUIRE(stopwatch.elapsed() == 0.0);

    // Call start on the Stopwatch and mock a 2 second passage of time,
    // making sure that the Stopwatch correctly handles it.
    stopwatch.start();
    REQUIRE(stopwatch.isRunning());
    stopwatch.advanceTime(2.0);
    stopwatch.stop();
    REQUIRE(stopwatch.elapsed() == Approx(2.0).margin(0.01));

    // Advance time while the Stopwatch is paused and make sure it doesn't increase the elapsed time.
    stopwatch.advanceTime(1.0);
    REQUIRE(stopwatch.elapsed() == Approx(2.0).margin(0.01));

    // Start the Stopwatch back up and advance time by some less conformative intervals.
    stopwatch.start();
    stopwatch.advanceTime(1.64);
    REQUIRE(stopwatch.elapsed() == Approx(3.64).margin(0.01));
    stopwatch.advanceTime(0.93);
    REQUIRE(stopwatch.elapsed() == Approx(4.57).margin(0.01));
}

TEST_CASE("Stopwatch Reset Method", "[Stopwatch]")
{
    // Start a Stopwatch, advance some time, stop the Stopwatch,
    // then call reset and make sure it's elapsed time is reset and state is unchanged.
    cse498::Stopwatch stopwatch("Test1");
    REQUIRE(stopwatch.isRunning());
    stopwatch.advanceTime(4.69);
    REQUIRE(stopwatch.elapsed() == Approx(4.69).margin(0.01));
    stopwatch.stop();
    REQUIRE(!stopwatch.isRunning());
    stopwatch.reset();
    REQUIRE(!stopwatch.isRunning());
    REQUIRE(stopwatch.elapsed() == 0.0);

    // Test that reset still functions properly when called on a running Stopwatch as opposed to a stopped one.
    cse498::Stopwatch stopwatch2("Test2");
    REQUIRE(stopwatch2.isRunning());
    stopwatch2.advanceTime(2.64);
    REQUIRE(stopwatch2.elapsed() == Approx(2.64).margin(0.01));
    stopwatch2.reset();
    REQUIRE(!stopwatch2.isRunning());
    REQUIRE(stopwatch2.elapsed() == 0.0);

    // Test that calling reset back to back doesn't mess anything up.
    stopwatch2.reset();
    REQUIRE(!stopwatch2.isRunning());
    REQUIRE(stopwatch2.elapsed() == 0.0);
}

TEST_CASE("Stopwatch Restart Method", "[Stopwatch]")
{
    // Start a Stopwatch, advance some time, stop the Stopwatch,
    // then call restart and make sure it's elapsed time is reset but it is started back up again.
    cse498::Stopwatch stopwatch("Test1");
    REQUIRE(stopwatch.isRunning());
    stopwatch.advanceTime(7.22);
    REQUIRE(stopwatch.elapsed() == Approx(7.22).margin(0.01));
    stopwatch.stop();
    REQUIRE(!stopwatch.isRunning());
    stopwatch.restart();
    REQUIRE(stopwatch.isRunning());
    REQUIRE(stopwatch.elapsed() == Approx(0.00).margin(0.01));

    // Test that restart still functions properly when called on a running Stopwatch as opposed to a stopped one.
    cse498::Stopwatch stopwatch2("Test2");
    REQUIRE(stopwatch2.isRunning());
    stopwatch2.advanceTime(8.15);
    REQUIRE(stopwatch2.elapsed() == Approx(8.15).margin(0.01));
    stopwatch2.restart();
    REQUIRE(stopwatch2.isRunning());
    REQUIRE(stopwatch2.elapsed() == Approx(0.00).margin(0.01));

    // Test that calling it again shortly after the first execution still has the same outcome.
    stopwatch2.restart();
    REQUIRE(stopwatch2.isRunning());
    REQUIRE(stopwatch2.elapsed() == Approx(0.00).margin(0.01));
}

TEST_CASE("Stopwatch Lap Method", "[Stopwatch]")
{
    // Start a Stopwatch and make sure that the first lap time is updating properly alongside the Stopwatch.
    cse498::Stopwatch stopwatch("Test1");
    REQUIRE(stopwatch.isRunning());
    stopwatch.stop();
    REQUIRE(stopwatch.getLaps().at(0) == Approx(stopwatch.elapsed()).margin(0.01));

    // Start the Stopwatch back up, advance the time a little bit, lap it,
    // make sure that another lap was added to the Stopwatch, and that the first one properly recorded time.
    stopwatch.start();
    stopwatch.advanceTime(0.45);
    stopwatch.lap();
    REQUIRE(stopwatch.getLaps().size() == 2);
    REQUIRE(stopwatch.getLaps().at(0) == Approx(0.45).margin(0.01));

    // Advance some more time and make sure that lapping more than once is handled properly
    // without messing with previous values.
    stopwatch.advanceTime(0.63);
    stopwatch.lap();
    REQUIRE(stopwatch.getLaps().size() == 3);
    REQUIRE(stopwatch.getLaps().at(0) == Approx(0.45).margin(0.01));
    REQUIRE(stopwatch.getLaps().at(1) == Approx(0.63).margin(0.01));
}