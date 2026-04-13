#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/DataLog.hpp"

/*
Test checks functions of the datalog with no samples
*/
TEST_CASE("Empty Log Test", "[tools]"){

    cse498::DataLog log;
    CHECK(log.Count() == 0);

    CHECK(log.DataSamples().empty());

    //Checks if there exists no values in the data log
    CHECK_FALSE(log.Min().has_value());
    CHECK_FALSE(log.Max().has_value());
    CHECK_FALSE(log.Mean().has_value());
    CHECK_FALSE(log.Median().has_value());
}

/*
Test checks the functions of a datalog with one sample
*/
TEST_CASE("Single Entry Test", "[tools]"){

    cse498::DataLog log;
    log.Add(10.1);

    CHECK(log.Count() == 1);
    CHECK(log.DataSamples().size() == log.Count());
    CHECK(log.DataSamples()[0].value == Approx(10.1));

    CHECK(log.Median().value() == Approx(10.1));
    CHECK(log.Mean().value() == Approx(10.1));
    CHECK(log.Min().value() == Approx(10.1));
    CHECK(log.Max().value() == Approx(10.1));

}

/*
Test checks the functions of the datalog with multiple samples
*/
TEST_CASE("Multiple Entry Test", "[tools]"){

    cse498::DataLog log;
    log.Add(10.0);
    log.Add(12.6);
    log.Add(14.1);
    log.Add(11.1);

    CHECK(log.Count() == 4);

    //Even samples median calculation
    CHECK(log.Median().value() == Approx(11.85));
    CHECK(log.Mean().value() == Approx(11.95));
    CHECK(log.Min().value() == Approx(10.0));
    CHECK(log.Max().value() == Approx(14.1));

}

/*
Test checks the clear function of the datalog
*/
TEST_CASE("Clear Test", "[tools]"){

    cse498::DataLog log;
    log.Add(1.0);
    log.Add(2.0);
    log.Add(3.0);

    CHECK(log.Count() == 3);

    log.Clear();

    CHECK(log.Count() == 0);
    CHECK(log.DataSamples().empty());
    CHECK(log.DataSamples().size() == log.Count());
    CHECK_FALSE(log.Min().has_value());
    CHECK_FALSE(log.Max().has_value());
    CHECK_FALSE(log.Mean().has_value());
    CHECK_FALSE(log.Median().has_value());
}

/*
Test checks the functions of the datalog with positive and negative values
*/
TEST_CASE("Negative Value Entry Test", "[tools]"){

    cse498::DataLog log;
    log.Add(10.0);
    log.Add(-12.6);
    log.Add(-14.1);
    log.Add(-11.1);
    log.Add(0);

    CHECK(log.Count() == 5);

    //Odd samples median calculation
    CHECK(log.Median().value() == Approx(-11.1));
    CHECK(log.Mean().value() == Approx(-5.56));
    CHECK(log.Min().value() == Approx(-14.1));
    CHECK(log.Max().value() == Approx(10.0));

}

/*
Test checks that the timestamps associated with the data values are in increasing order
*/
TEST_CASE("Timestamp order check", "[tools]"){

    cse498::DataLog log;
    log.Add(1.0);
    log.Add(2.0);
    log.Add(3.0);

    CHECK(log.DataSamples().size() == 3);
    CHECK(log.DataSamples()[2].timestamp >= log.DataSamples()[1].timestamp);
    CHECK(log.DataSamples()[1].timestamp >= log.DataSamples()[0].timestamp);
    CHECK(log.DataSamples()[0].timestamp >= 0);
}

/*
Test checks that the initial timestamp value
*/
TEST_CASE("Initial Timestamp Test", "[tools]"){

    cse498::DataLog log;
    log.Add(1.0);


    CHECK(log.DataSamples().size() == 1);
    CHECK(log.DataSamples()[0].timestamp >= 0.0);
    CHECK(log.DataSamples()[0].timestamp < 0.1);
}

/*
Test checks that the initial Under/Over Threshold Timestamp value
*/
TEST_CASE("Empty Threshold Test", "[tools]"){

    cse498::DataLog log;

    CHECK(log.TimeUnderThreshold(1.0) == 0.0);
    CHECK(log.TimeOverThreshold(1.0) == 0.0);
}

/*
Test checks Under Threshold Timestamp value
*/
TEST_CASE("Multiple Value TimeUnderThreshold Test", "[tools]"){

    cse498::DataLog log;

    log.Add(10.0);
    log.advanceTimeForTesting(10.0);
    log.Add(20.0);

    CHECK(log.TimeUnderThreshold(15.0) == Approx(10.0));

}

/*
Test checks Over Threshold Timestamp value
*/
TEST_CASE("Multiple Value TimeOverThreshold Test", "[tools]"){

    cse498::DataLog log;

    log.Add(20.0);
    log.advanceTimeForTesting(2.0);
    log.Add(10.0);

    CHECK(log.TimeOverThreshold(15.0) == Approx(2.0));

}

/*
Test checks single sample threshold time value
*/
TEST_CASE("Single Value Threshold Time Test", "[tools]"){

    cse498::DataLog log;

    log.Add(20.0);
    log.advanceTimeForTesting(10.0);

    CHECK(log.TimeOverThreshold(10.0) == Approx(0.0));
    CHECK(log.TimeUnderThreshold(30.0) == Approx(0.0));

}

/*
Test checks that final sample is not added to threshold time calculations
*/
TEST_CASE("Final Sample Time Inclusion Test", "[tools]"){

    cse498::DataLog log;

    log.Add(20.0);
    log.advanceTimeForTesting(5.0);
    log.Add(10.0);
    log.advanceTimeForTesting(4.0);

    CHECK(log.TimeOverThreshold(15.0) == Approx(5.0));
    CHECK(log.TimeUnderThreshold(15.0) == Approx(0.0));

}