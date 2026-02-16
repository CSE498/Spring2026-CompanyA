#include "FeatureVector.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using cse498::FeatureVector;

TEST_CASE("FeatureVector dot product", "[dot]")
{
    FeatureVector v1 ({1.0, 2.0, 3.0});

    FeatureVector v2 ({4.0, 5.0, 6.0});

    REQUIRE(v1.dot(v2) == 32.0);
}

TEST_CASE("FeatureVector normalize", "[normalize]")
{
    FeatureVector v({3.0, 4.0});
    v.normalize();

    REQUIRE_THAT(v.at(0), Catch::Matchers::WithinAbs(0.6, 1e-9));

    REQUIRE_THAT(v.at(1), Catch::Matchers::WithinAbs(0.8, 1e-9));
}

TEST_CASE("FeatureVector at function", "[at]")
{
    FeatureVector v ({5.0, 12.0, 9.0});

    REQUIRE(v.at(0) == 5.0);
    REQUIRE(v.at(1) == 12.0);
    REQUIRE(v.at(2) == 9.0);
}

TEST_CASE("FeatureVector at functions throws on out of range", "[at]")
{
    FeatureVector v ({9.0, 5.0});
    REQUIRE_THROWS_AS(v.at(7), std::out_of_range);

}


TEST_CASE("FeatureVector dot product with itself", "[dot]")
{
    FeatureVector v ({5.0, 12.0});
    REQUIRE(v.dot(v) == 169.0);
}