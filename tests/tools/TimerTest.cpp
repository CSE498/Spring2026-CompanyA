#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include <thread>

#include "../../source/tools/Timer.hpp"

TEST_CASE("Test Timer Constructor and Basic Functionality", "[tools]")
{
    cse498::Timer timer;

    // Test that a new timer can be started and that its time is approximately 0 seconds.
    timer.start("test1");
    REQUIRE(timer.isRunning("test1") == true); // Timer should be running after starting.
    auto duration1 = timer.getTime("test1");
    REQUIRE(duration1 == Approx(0.0).margin(0.01)); // Should be close to 0 seconds.

    // Test that a timer that doesn't exist returns a time of 0 seconds, and is marked as not running.
    auto durationFake = timer.getTime("fakeTimer");
    REQUIRE(durationFake == 0.0);                   // Should be exactly 0 seconds for a non-existent timer.
    REQUIRE(timer.isRunning("fakeTimer") == false); // Should be marked as not running.

    // Test that the timer accumulates time correctly after running for a short duration.
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Sleep for 2 seconds.
    auto duration2 = timer.getTime("test1");
    REQUIRE(duration2 == Approx(2.0).margin(0.01)); // Should be close to 2 seconds.

    // Test that stopping the timer returns the correct time and is correctly marked as stopped.
    timer.stop("test1");
    REQUIRE(timer.isRunning("test1") == false); // Timer should be stopped after stopping.
    auto duration3 = timer.getTime("test1");
    REQUIRE(duration3 == Approx(2.0).margin(0.01)); // Should still be close to 2 seconds after stopping.

    // Test that the timer's duration does not increase after being stopped.
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Sleep for 1 second.
    auto duration4 = timer.getTime("test1");
    REQUIRE(duration4 == Approx(2.0).margin(0.01)); // Should still be close to 2 seconds, not 3 seconds.

    // Test that restarting the timer resets its duration to 0 and starts it again.
    timer.restart("test1");
    REQUIRE(timer.isRunning("test1") == true); // Timer should be running after restarting.
    auto duration5 = timer.getTime("test1");
    REQUIRE(duration5 == Approx(0.0).margin(0.01));       // Should be close to 0 seconds after restarting.
    std::this_thread::sleep_for(std::chrono::seconds(4)); // Sleep for 4 seconds.
    auto duration6 = timer.getTime("test1");
    REQUIRE(duration6 == Approx(4.0).margin(0.01)); // Should be close to 4 seconds after sleeping for 4 seconds.
}