/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * It is apart of Green and White Games (Company A) Group 15's module.
 * @brief A test file for Random.hpp
 * @note Status: PROPOSAL
 **/

#include "../../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../../source/tools/Random.hpp"

#include <vector>
#include <algorithm>
#include <iostream>


TEST_CASE("Test Seed Setting", "[core]") {
    cse498::Random ran1;
    cse498::Random ran2;
    uint64_t generatedSeed = ran2.GetSeed();
    ran2.SetSeed(1000);   // Manually Set the seed
    CHECK(ran2.GetSeed() != generatedSeed);
    CHECK(ran2.GetSeed() == 1000);
}

TEST_CASE("Test Generation Happens", "[core]") {
    cse498::Random ran4;

    // int
    std::vector<int> generations_int;
    for (int i = 0; i < 10; ++i) {
        generations_int.push_back(ran4.GetInt());
    }
    REQUIRE(generations_int.size() == 10);
    
    // double
    std::vector<double> generations_double;
    for (int i = 0; i < 10; ++i) {
        generations_double.push_back(ran4.GetDouble());
    }
    REQUIRE(generations_double.size() == 10);

    // float
    std::vector<float> generations_float;
    for (int i = 0; i < 10; ++i) {
        generations_float.push_back(ran4.GetFloat());
    }
    REQUIRE(generations_float.size() == 10);

    // char
    std::vector<char> generations_char;
    for (int i = 0; i < 10; ++i) {
        generations_char.push_back(ran4.GetChar());
    }
    REQUIRE(generations_char.size() == 10);

    // bool
    std::vector<bool> generations_bool;
    for (int i = 0; i < 10; ++i) {
        generations_bool.push_back(ran4.GetBool());
    }
    REQUIRE(generations_bool.size() == 10);

    // p
    std::vector<bool> generations_p;
    for (int i = 0; i < 10; ++i) {
        generations_p.push_back(ran4.P());
    }
    REQUIRE(generations_p.size() == 10);
}

TEST_CASE("Test Random Generation", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    std::vector<int> seed1000 = {60,85,45,71,60,100,16,61,1,50};
    cse498::Random ran4;
    ran4.SetSeed(1000);

    /// Checking int generation
    // With default range
    std::vector<int> generations_int;
    for (int i = 0; i < 10; ++i) {
        generations_int.push_back(ran4.GetInt());
    }
    REQUIRE(generations_int.size() == seed1000.size());
    REQUIRE(seed1000 == generations_int);
    
    // With large range
    seed1000 = {1827427, 6162588, 21128328, 93315334, 28331346, 12688761, 70148246, 77165926, 87193757, 83189719};
    std::vector<int> generations_int_large;
    for (int i = 0; i < 10; ++i) {
        generations_int_large.push_back(ran4.GetInt(0, 100000000));
    }
    REQUIRE(generations_int_large.size() == seed1000.size());
    REQUIRE(seed1000 == generations_int_large);

    // With small range
    seed1000 = {1, 3, 3, 2, 1, 1, 1, 1, 3, 1};
    std::vector<int> generations_int_small;
    for (int i = 0; i < 10; ++i) {
        generations_int_small.push_back(ran4.GetInt(1, 3));
    }
    REQUIRE(generations_int_small.size() == seed1000.size());
    REQUIRE(seed1000 == generations_int_small);

    // With an error range (ill-formed parameter test cases)
    CHECK_THROWS(ran4.GetInt(100,1));

    // With negative numbers
    seed1000 = {-60, -72, 69, -21, 86, -91, 47, -33, 9, 84};
    std::vector<int> generations_int_neg;
    for (int i = 0; i < 10; ++i) {
        generations_int_neg.push_back(ran4.GetInt(-100, 100));
    }
    REQUIRE(generations_int_neg.size() == seed1000.size());
    REQUIRE(seed1000 == generations_int_neg);

    // testing different seeds produce different numbers with a seed that is gauranteed to be different numbers
    cse498::Random ran5;
    ran5.SetSeed(1111);
    std::vector<int> generations_int_2;
    for (int i = 0; i < 10; ++i) {
        generations_int_2.push_back(ran4.GetInt());
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
    for (int i = 0; i < 1000; ++i) {
        generations_int_1.push_back(ran15.GetInt());
        generations_int_2.push_back(ran16.GetInt());
    }
    REQUIRE(generations_int_1.size() == generations_int_2.size());
    CHECK(generations_int_1 == generations_int_2);

    // With large range
    std::vector<int> generations_int_large_1;
    std::vector<int> generations_int_large_2;
    for (int i = 0; i < 1000; ++i) {
        generations_int_large_1.push_back(ran15.GetInt(0, 100000000));
        generations_int_large_2.push_back(ran16.GetInt(0, 100000000));
    }
    REQUIRE(generations_int_large_1.size() == generations_int_large_2.size());
    CHECK(generations_int_large_1 == generations_int_large_2);

    // With small range
    std::vector<int> generations_int_small_1;
    std::vector<int> generations_int_small_2;
    for (int i = 0; i < 1000; ++i) {
        generations_int_small_1.push_back(ran15.GetInt(1, 3));
        generations_int_small_2.push_back(ran16.GetInt(1, 3));
    }
    REQUIRE(generations_int_small_1.size() == generations_int_small_2.size());
    CHECK(generations_int_small_1 == generations_int_small_2);

    // With negative numbers
    std::vector<int> generations_int_neg_1;
    std::vector<int> generations_int_neg_2;
    for (int i = 0; i < 1000; ++i) {
        generations_int_neg_1.push_back(ran15.GetInt(-100, 100));
        generations_int_neg_2.push_back(ran16.GetInt(-100, 100));
    }
    REQUIRE(generations_int_neg_1.size() == generations_int_neg_2.size());
    CHECK(generations_int_neg_1 == generations_int_neg_2);
}