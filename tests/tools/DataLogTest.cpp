#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/DataLog.hpp"

/*
Test checks functions of the datalog with no samples
*/
TEST_CASE("Empty Log Test", "[tools]"){

    DataLog log;
    CHECK(log.Count() == 0);

    CHECK(log.DataSamples().empty());
    CHECK(log.DataSamples().size() == log.Count());

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

/*
Test checks the functions of a datalog with one sample
*/
TEST_CASE("Single Entry Test", "[tools]"){

    DataLog log;
    log.Add(10.1);

    CHECK(log.Count() == 1);
    CHECK(log.DataSamples().size() == log.Count());
    CHECK(log.DataSamples()[0].first == Approx(10.1));

    CHECK(log.Median().value() == Approx(10.1));
    CHECK(log.Mean().value() == Approx(10.1));
    CHECK(log.Min().value() == Approx(10.1));
    CHECK(log.Max().value() == Approx(10.1));

    log.Clear();

    CHECK(log.Count() == 0);
    CHECK(log.DataSamples().empty());
    CHECK(log.DataSamples().size() == log.Count());

}

/*
Test checks the functions of the datalog with multiple samples
*/
TEST_CASE("Multiple Entry Test", "[tools]"){

    DataLog log;
    log.Add(10.0);
    log.Add(12.6);
    log.Add(14.1);
    log.Add(11.1);

    CHECK(log.Count() == 4);
    CHECK(log.Median().value() == Approx(11.85));
    CHECK(log.Mean().value() == Approx(11.95));
    CHECK(log.Min().value() == Approx(10.0));
    CHECK(log.Max().value() == Approx(14.1));

    log.Clear();

    CHECK(log.Count() == 0);
    CHECK(log.DataSamples().empty());
    CHECK(log.DataSamples().size() == log.Count());
}

/*
Test checks the functions of the datalog with positive and negative values
*/
TEST_CASE("Negative Value Entry Test", "[tools]"){

    DataLog log;
    log.Add(10.0);
    log.Add(-12.6);
    log.Add(-14.1);
    log.Add(-11.1);
    log.Add(0);

    CHECK(log.Count() == 5);
    CHECK(log.Median().value() == Approx(-11.1));
    CHECK(log.Mean().value() == Approx(-5.56));
    CHECK(log.Min().value() == Approx(-14.1));
    CHECK(log.Max().value() == Approx(10.0));

    log.Clear();

    CHECK(log.Count() == 0);
    CHECK(log.DataSamples().empty());
    CHECK(log.DataSamples().size() == log.Count());
}

/*
Test checks that the timestamps associated with the data values are in increasing order
*/
TEST_CASE("Timestamp order check", "[tools]"){

    DataLog log;
    log.Add(1.0);
    log.Add(2.0);
    log.Add(3.0);

    CHECK(log.DataSamples().size() == 3);
    CHECK(log.DataSamples()[2].second >= log.DataSamples()[1].second);
    CHECK(log.DataSamples()[1].second >= log.DataSamples()[0].second);
    CHECK(log.DataSamples()[0].second >= 0);
}
