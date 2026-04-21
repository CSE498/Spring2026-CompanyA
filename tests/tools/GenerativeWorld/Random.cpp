/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * It is apart of Green and White Games (Company A) Group 15's module.
 * @brief A test file for Random.hpp
 * @note Status: PROPOSAL
 **/

#include "../../../source/tools/Random.hpp"
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include <algorithm>
#include <iostream>
#include <vector>


TEST_CASE("Test Seed Setting", "[core]") {
    cse498::Random ran1;
    cse498::Random ran2;
    uint64_t generatedSeed = ran2.GetSeed();
    ran2.SetSeed(1000); // Manually Set the seed
    CHECK(ran2.GetSeed() != generatedSeed);
    CHECK(ran2.GetSeed() == 1000);
}

TEST_CASE("Test Error Handling", "[core]") {
    cse498::Random ran3;

    auto error_num1 = ran3.GetValue(10, 1);
    REQUIRE_FALSE(error_num1.has_value());
    REQUIRE(error_num1.error() == "cse498::Random::GetValue(): min must be less than or equal to max.");

    auto error_num2 = ran3.P(-1);
    REQUIRE_FALSE(error_num2.has_value());
    REQUIRE(error_num2.error() == "cse498::Random::P(): probability must be greater than or equal to 0");

    auto error_num3 = ran3.P(2);
    REQUIRE_FALSE(error_num3.has_value());
    REQUIRE(error_num3.error() == "cse498::Random::P(): probability must be less than or equal to 1");
}

TEST_CASE("Test Generation Happens", "[core]") {
    cse498::Random ran4;

    // int
    std::vector<int> generations_int;
    for (int i = 0; i < 10; ++i) {
        generations_int.push_back(ran4.GetValue(1, 10).value());
    }
    REQUIRE(generations_int.size() == 10);

    // double
    std::vector<double> generations_double;
    for (int i = 0; i < 10; ++i) {
        generations_double.push_back(ran4.GetValue(1.0, 10.0).value());
    }
    REQUIRE(generations_double.size() == 10);

    // float
    std::vector<float> generations_float;
    for (int i = 0; i < 10; ++i) {
        generations_float.push_back(ran4.GetValue(1.0f, 10.0f).value());
    }
    REQUIRE(generations_float.size() == 10);

    // char
    std::vector<char> generations_char;
    for (int i = 0; i < 10; ++i) {
        generations_char.push_back(ran4.GetValue('a', 'z').value());
    }
    REQUIRE(generations_char.size() == 10);

    // bool
    std::vector<bool> generations_bool;
    for (int i = 0; i < 10; ++i) {
        generations_bool.push_back(ran4.GetValue(false, true).value());
    }
    REQUIRE(generations_bool.size() == 10);

    // p
    std::vector<bool> generations_p;
    for (int i = 0; i < 10; ++i) {
        generations_p.push_back(ran4.P().value());
    }
    REQUIRE(generations_p.size() == 10);
}

TEST_CASE("Test Random Generation", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    std::vector<int> seed1000 = {10, 6, 9, 6, 7, 4, 7, 5, 3, 1};
    cse498::Random ran4;
    ran4.SetSeed(1000);

    /// Checking int generation
    // With regular range
    std::vector<int> generations_int;
    for (int i = 0; i < 10; ++i) {
        generations_int.push_back(ran4.GetValue(1, 10).value());
    }
    REQUIRE(generations_int.size() == seed1000.size());
    REQUIRE(seed1000 == generations_int);

    // With large range
    seed1000 = {96755444, 6877909, 27446121, 27988652, 21206584, 2012321, 5775199, 94442425, 46733563, 64994790};
    std::vector<int> generations_int_large;
    for (int i = 0; i < 10; ++i) {
        generations_int_large.push_back(ran4.GetValue(0, 100000000).value());
    }
    REQUIRE(generations_int_large.size() == seed1000.size());
    REQUIRE(seed1000 == generations_int_large);

    // With small range
    seed1000 = {1, 2, 3, 1, 1, 1, 1, 3, 1, 1};
    std::vector<int> generations_int_small;
    for (int i = 0; i < 10; ++i) {
        generations_int_small.push_back(ran4.GetValue(1, 3).value());
    }
    REQUIRE(generations_int_small.size() == seed1000.size());
    REQUIRE(seed1000 == generations_int_small);

    // With negative numbers
    seed1000 = {25, -79, 12, 13, -79, -19, -68, -6, -17, 6};
    std::vector<int> generations_int_neg;
    for (int i = 0; i < 10; ++i) {
        generations_int_neg.push_back(ran4.GetValue(-100, 100).value());
    }
    REQUIRE(generations_int_neg.size() == seed1000.size());
    REQUIRE(seed1000 == generations_int_neg);

    // testing different seeds produce different numbers with a seed that is gauranteed to be different numbers
    cse498::Random ran5;
    ran5.SetSeed(1111);
    std::vector<int> generations_int_2;
    for (int i = 0; i < 10; ++i) {
        generations_int_2.push_back(ran4.GetValue(1, 10).value());
    }
    REQUIRE(generations_int.size() == generations_int_2.size());
    REQUIRE(generations_int_2 != generations_int);
}

TEST_CASE("Test Random Integer Generation Reproducability with Same Seed", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    cse498::Random ran15;
    ran15.SetSeed(1000);
    cse498::Random ran16;
    ran16.SetSeed(1000);

    /// Checking int generation
    // With default range
    std::vector<int> generations_int_1;
    std::vector<int> generations_int_2;
    for (int i = 0; i < 10; ++i) {
        generations_int_1.push_back(ran15.GetValue(1, 10).value());
        generations_int_2.push_back(ran16.GetValue(1, 10).value());
    }
    REQUIRE(generations_int_1.size() == generations_int_2.size());
    CHECK(generations_int_1 == generations_int_2);

    // With large range
    std::vector<int> generations_int_large_1;
    std::vector<int> generations_int_large_2;
    for (int i = 0; i < 10; ++i) {
        generations_int_large_1.push_back(ran15.GetValue(0, 100000000).value());
        generations_int_large_2.push_back(ran16.GetValue(0, 100000000).value());
    }
    REQUIRE(generations_int_large_1.size() == generations_int_large_2.size());
    CHECK(generations_int_large_1 == generations_int_large_2);

    // With small range
    std::vector<int> generations_int_small_1;
    std::vector<int> generations_int_small_2;
    for (int i = 0; i < 10; ++i) {
        generations_int_small_1.push_back(ran15.GetValue(1, 3).value());
        generations_int_small_2.push_back(ran16.GetValue(1, 3).value());
    }
    REQUIRE(generations_int_small_1.size() == generations_int_small_2.size());
    CHECK(generations_int_small_1 == generations_int_small_2);

    // With negative numbers
    std::vector<int> generations_int_neg_1;
    std::vector<int> generations_int_neg_2;
    for (int i = 0; i < 10; ++i) {
        generations_int_neg_1.push_back(ran15.GetValue(-100, 100).value());
        generations_int_neg_2.push_back(ran16.GetValue(-100, 100).value());
    }
    REQUIRE(generations_int_neg_1.size() == generations_int_neg_2.size());
    CHECK(generations_int_neg_1 == generations_int_neg_2);
}
