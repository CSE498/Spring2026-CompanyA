#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/DataLog.hpp"

TEST_CASE("Initial Test", "[tools]"){

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

TEST_CASE("Empty Log Test", "[tools]"){

    DataLog log;
    CHECK(log.Count() == 0);

    //Checks if the error msg is returned when a log is empty
    CHECK(log.Min().error() == "Data is empty.");
    CHECK(log.Max().error() == "Data is empty.");
    CHECK(log.Mean().error() == "Data is empty.");
    CHECK(log.Median().error() == "Data is empty.");

    //Checks if the error msg is returned
    CHECK_FALSE(log.Min().has_value());
    CHECK_FALSE(log.Max().has_value());
    CHECK_FALSE(log.Mean().has_value());
    CHECK_FALSE(log.Median().has_value());

    log.Clear();

    CHECK(log.Count() == 0);

}

TEST_CASE("Single Entry Test", "[tools]"){

    DataLog log;
    log.Add(10.1,1.0);

    CHECK(log.Count() == 1);
    CHECK(log.Median() == 10.1);
    CHECK(log.Mean() == 10.1);
    CHECK(log.Min() == 10.1);
    CHECK(log.Max() == 10.1);

    log.Clear();

    CHECK(log.Count() == 0);

}

TEST_CASE("Multiple Entry Test", "[tools]"){

    DataLog log;
    log.Add(10.0,1.0);
    log.Add(12.6,2.0);
    log.Add(14.1,5.0);
    log.Add(11.1,6.0);

    CHECK(log.Count() == 4);
    CHECK(log.Median() == Approx(11.85));
    CHECK(log.Mean() == Approx(11.95));
    CHECK(log.Min() == 10.0);
    CHECK(log.Max() == 14.1);

    log.Clear();

    CHECK(log.Count() == 0);
}

TEST_CASE("Negative Value Entry Test", "[tools]"){

    DataLog log;
    log.Add(10.0,1.0);
    log.Add(-12.6,2.0);
    log.Add(-14.1,5.0);
    log.Add(-11.1,8.0);
    log.Add(0,9.0);

    CHECK(log.Count() == 5);
    CHECK(log.Median() == Approx(-11.1));
    CHECK(log.Mean() == Approx(-5.56));
    CHECK(log.Min() == -14.1);
    CHECK(log.Max() == 10.0);

    log.Clear();

    CHECK(log.Count() == 0);
}

TEST_CASE("Invalid Timestamp Impact", "[tools]"){

    DataLog log;
    log.Add(1.0,1000);
    log.Add(2.0, -1000);

    CHECK(log.Count() == 2);
    CHECK(log.Median() == Approx(1.5));
    CHECK(log.Mean() == Approx(1.5));
    CHECK(log.Min() == 1.0);
    CHECK(log.Max() == 2.0);

    log.Clear();

    CHECK(log.Count() == 0);
}

