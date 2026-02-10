#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/Timer.hpp"

TEST_CASE("Test Timer Constructor and Basic Functionality", "[tools]")
{
    Timer timer;

    // Start a timer and immediately stop it, should be close to 0 seconds.
    timer.start("testTimer");
    auto duration = timer.stop("testTimer");
    CHECK(duration.count() >= 0);
    CHECK(duration.count() < 0.1); // Allow for some small overhead.

    // Test stopping a non-existent timer, should return 0 seconds.
    auto nonExistentDuration = timer.stop("nonExistentTimer");
    CHECK(nonExistentDuration.count() == 0);
}