#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/DataLog.hpp"


TEST_CASE("Initial Test"){

    DataLog log;
    CHECK(log.Count() == 0);

    log.Add(10.0,0.0);
    log.Add(12.0,1.0);
    log.Add(14.0,2.0);
    log.Add(16.0,3.0);
    log.Add(18.0,4.0);

    CHECK(log.Count() == 5);
    CHECK(log.Median() == 14);
    CHECK(log.Mean() == 14);
    CHECK(log.Min() == 10);
    CHECK(log.Max() == 18);

    log.Clear();

    CHECK(log.Count() == 0);
    CHECK(log.Min().error() == "Data is empty.");
    CHECK(log.Max().error() == "Data is empty.");
    CHECK(log.Mean().error() == "Data is empty.");
    CHECK(log.Median().error() == "Data is empty.");
 
}