/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * It is apart of Green and White Games (Company A) Group 15's module.
 * @brief A test file for Random.hpp
 * @note Status: PROPOSAL
 **/

#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../../source/tools/Random.hpp"
#include <vector>


TEST_CASE("Test Seed Setting", "[core]") {
    cse498::Random ran1;
    cse498::Random ran2;
    uint64_t generatedSeed = ran2.GetSeed();
    ran2.SetSeed(1000);   // Manually Set the seed
    CHECK(ran2.GetSeed() != generatedSeed);
    CHECK(ran2.GetSeed() == 1000);
}

TEST_CASE("Test Random Generation with Set Seed", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    cse498::Random ran4;
    ran4.SetSeed(1000);

    /// Checking int generation
    // With default range
    std::vector<int> generations_int;
    for (int i = 0; i < 1000; ++i) {
        generations_int.push_back(ran4.GetInt());
    }
    REQUIRE(generations_int.size() >= 2);
    CHECK(std::any_of(generations_int.begin() + 1, generations_int.end(), [&](const auto& x) { return x != generations_int[0]; }));

    // With large range
    std::vector<int> generations_int_large;
    for (int i = 0; i < 1000; ++i) {
        generations_int_large.push_back(ran4.GetInt(0, 100000000));
    }
    REQUIRE(generations_int_large.size() >= 2);
    CHECK(std::any_of(generations_int_large.begin() + 1, generations_int_large.end(), [&](const auto& x) { return x != generations_int_large[0]; }));

    // With small range
    std::vector<int> generations_int_small;
    for (int i = 0; i < 1000; ++i) {
        generations_int_small.push_back(ran4.GetInt(1, 3));
    }
    REQUIRE(generations_int_small.size() >= 2);
    CHECK(std::any_of(generations_int_small.begin() + 1, generations_int_small.end(), [&](const auto& x) { return x != generations_int_small[0]; }));

    // With an error range
    CHECK_THROWS(ran4.GetInt(100,1));

    // With negative numbers
    std::vector<int> generations_int_neg;
    for (int i = 0; i < 1000; ++i) {
        generations_int_neg.push_back(ran4.GetInt(-100, 100));
    }
    REQUIRE(generations_int_neg.size() >= 2);
    CHECK(std::any_of(generations_int_neg.begin() + 1, generations_int_neg.end(), [&](const auto& x) { return x != generations_int_neg[0]; }));



    /// Checking double generation
    // With default range
    std::vector<double> generations_double;
    for (int i = 0; i < 1000; ++i) {
        generations_double.push_back(ran4.GetDouble());
    }
    REQUIRE(generations_double.size() >= 2);
    CHECK(std::any_of(generations_double.begin() + 1, generations_double.end(), [&](const auto& x) { return x != generations_double[0]; }));

    // With large range
    std::vector<int> generations_double_large;
    for (int i = 0; i < 1000; ++i) {
        generations_double_large.push_back(ran4.GetDouble(0.0, 100000000.9));
    }
    REQUIRE(generations_double_large.size() >= 2);
    CHECK(std::any_of(generations_double_large.begin() + 1, generations_double_large.end(), [&](const auto& x) { return x != generations_double_large[0]; }));

    // With small range
    std::vector<double> generations_double_small;
    for (int i = 0; i < 1000; ++i) {
        generations_double_small.push_back(ran4.GetDouble(0.1, 0.8));
    }
    REQUIRE(generations_double_small.size() >= 2);
    CHECK(std::any_of(generations_double_small.begin() + 1, generations_double_small.end(), [&](const auto& x) { return x != generations_double_small[0]; }));

    // With an error range
    CHECK_THROWS(ran4.GetDouble(100.9,1.0));

    // With negative numbers
    std::vector<int> generations_double_neg;
    for (int i = 0; i < 1000; ++i) {
        generations_double_neg.push_back(ran4.GetDouble(-100.9, 100.8));
    }
    REQUIRE(generations_double_neg.size() >= 2);
    CHECK(std::any_of(generations_double_neg.begin() + 1, generations_double_neg.end(), [&](const auto& x) { return x != generations_double_neg[0]; }));



    // Checking float generation
    // With default range
    std::vector<float> generations_float;
    for (int i = 0; i < 1000; ++i) {
        generations_float.push_back(ran4.GetFloat());
    }
    REQUIRE(generations_float.size() >= 2);
    CHECK(std::any_of(generations_float.begin() + 1, generations_float.end(), [&](const auto& x) { return x != generations_float[0]; }));
    
    // With large range
    std::vector<float> generations_float_large;
    for (int i = 0; i < 1000; ++i) {
        generations_float_large.push_back(ran4.GetFloat(0.01, 100000000.92));
    }
    REQUIRE(generations_float_large.size() >= 2);
    CHECK(std::any_of(generations_float_large.begin() + 1, generations_float_large.end(), [&](const auto& x) { return x != generations_float_large[0]; }));

    // With small range
    std::vector<float> generations_float_small;
    for (int i = 0; i < 1000; ++i) {
        generations_float_small.push_back(ran4.GetFloat(0.13, 0.89));
    }
    REQUIRE(generations_double_small.size() >= 2);
    CHECK(std::any_of(generations_float_small.begin() + 1, generations_float_small.end(), [&](const auto& x) { return x != generations_float_small[0]; }));

    // With an error range
    CHECK_THROWS(ran4.GetFloat(100.99,1.04));

    // With negative numbers
    std::vector<float> generations_float_neg;
    for (int i = 0; i < 1000; ++i) {
        generations_float_neg.push_back(ran4.GetFloat(-100.94, 100.86));
    }
    REQUIRE(generations_float_neg.size() >= 2);
    CHECK(std::any_of(generations_float_neg.begin() + 1, generations_float_neg.end(), [&](const auto& x) { return x != generations_float_neg[0]; }));



    // Checking char generation
    // With default range
    std::vector<char> generations_char;
    for (int i = 0; i < 1000; ++i) {
        generations_char.push_back(ran4.GetChar());
    }
    REQUIRE(generations_char.size() >= 2);
    CHECK(std::any_of(generations_char.begin() + 1, generations_char.end(), [&](const auto& x) { return x != generations_char[0]; }));

    // With custom range
    std::vector<char> generations_char_large;
    for (int i = 0; i < 1000; ++i) {
        generations_char_large.push_back(ran4.GetChar('B', 'G'));
    }
    REQUIRE(generations_char_large.size() >= 2);
    CHECK(std::any_of(generations_char_large.begin() + 1, generations_char_large.end(), [&](const auto& x) { return x != generations_char_large[0]; }));

    // With small range
    std::vector<char> generations_char_small;
    for (int i = 0; i < 1000; ++i) {
        generations_char_small.push_back(ran4.GetChar('A', 'B'));
    }
    REQUIRE(generations_char_small.size() >= 2);
    CHECK(std::any_of(generations_char_small.begin() + 1, generations_char_small.end(), [&](const auto& x) { return x != generations_char_small[0]; }));

    // With an error range
    CHECK_THROWS(ran4.GetChar('Z','A'));

    

    // Checking bool generation
    std::vector<bool> generations_bool;
    for (int i = 0; i < 1000; ++i) {
        generations_bool.push_back(ran4.GetBool());
    }
    REQUIRE(generations_bool.size() >= 2);
    CHECK(std::any_of(generations_bool.begin() + 1, generations_bool.end(), [&](const auto& x) { return x != generations_bool[0]; }));
    


    // Checking probability generation
    // With default range
    std::vector<bool> generations_p;
    for (int i = 0; i < 1000; ++i) {
        generations_p.push_back(ran4.P());
    }
    REQUIRE(generations_p.size() >= 2);
    CHECK(std::any_of(generations_p.begin() + 1, generations_p.end(), [&](const auto& x) { return x != generations_p[0]; }));

    // With small range
    std::vector<bool> generations_p_small;
    for (int i = 0; i < 1000; ++i) {
        generations_p_small.push_back(ran4.P(0.1));
    }
    REQUIRE(generations_p_small.size() >= 2);
    CHECK(std::any_of(generations_p_small.begin() + 1, generations_p_small.end(), [&](const auto& x) { return x != generations_p_small[0]; }));

    // With large range
    std::vector<bool> generations_p_large;
    for (int i = 0; i < 1000; ++i) {
        generations_p_large.push_back(ran4.P(0.9));
    }
    REQUIRE(generations_p_large.size() >= 2);
    CHECK(std::any_of(generations_p_large.begin() + 1, generations_p_large.end(), [&](const auto& x) { return x != generations_p_large[0]; }));

    // With small error range
    CHECK_THROWS(ran4.P(-1));

    // With large error range
    CHECK_THROWS(ran4.P(1.1));
}

TEST_CASE("Test Random Generation with Generated Seed", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    cse498::Random ran5;

    /// Checking int generation
    // With default range
    std::vector<int> generations_int;
    for (int i = 0; i < 1000; ++i) {
        generations_int.push_back(ran5.GetInt());
    }
    REQUIRE(generations_int.size() >= 2);
    CHECK(std::any_of(generations_int.begin() + 1, generations_int.end(), [&](const auto& x) { return x != generations_int[0]; }));

    // With large range
    std::vector<int> generations_int_large;
    for (int i = 0; i < 1000; ++i) {
        generations_int_large.push_back(ran5.GetInt(0, 100000000));
    }
    REQUIRE(generations_int_large.size() >= 2);
    CHECK(std::any_of(generations_int_large.begin() + 1, generations_int_large.end(), [&](const auto& x) { return x != generations_int_large[0]; }));

    // With small range
    std::vector<int> generations_int_small;
    for (int i = 0; i < 1000; ++i) {
        generations_int_small.push_back(ran5.GetInt(1, 3));
    }
    REQUIRE(generations_int_small.size() >= 2);
    CHECK(std::any_of(generations_int_small.begin() + 1, generations_int_small.end(), [&](const auto& x) { return x != generations_int_small[0]; }));

    // With an error range
    CHECK_THROWS(ran5.GetInt(100,1));

    // With negative numbers
    std::vector<int> generations_int_neg;
    for (int i = 0; i < 1000; ++i) {
        generations_int_neg.push_back(ran5.GetInt(-100, 100));
    }
    REQUIRE(generations_int_neg.size() >= 2);
    CHECK(std::any_of(generations_int_neg.begin() + 1, generations_int_neg.end(), [&](const auto& x) { return x != generations_int_neg[0]; }));



    /// Checking double generation
    // With default range
    std::vector<double> generations_double;
    for (int i = 0; i < 1000; ++i) {
        generations_double.push_back(ran5.GetDouble());
    }
    REQUIRE(generations_double.size() >= 2);
    CHECK(std::any_of(generations_double.begin() + 1, generations_double.end(), [&](const auto& x) { return x != generations_double[0]; }));

    // With large range
    std::vector<double> generations_double_large;
    for (int i = 0; i < 1000; ++i) {
        generations_double_large.push_back(ran5.GetDouble(0.0, 100000000.9));
    }
    REQUIRE(generations_double_large.size() >= 2);
    CHECK(std::any_of(generations_double_large.begin() + 1, generations_double_large.end(), [&](const auto& x) { return x != generations_double_large[0]; }));

    // With small range
    std::vector<double> generations_double_small;
    for (int i = 0; i < 1000; ++i) {
        generations_double_small.push_back(ran5.GetDouble(0.1, 0.8));
    }
    REQUIRE(generations_double_small.size() >= 2);
    CHECK(std::any_of(generations_double_small.begin() + 1, generations_double_small.end(), [&](const auto& x) { return x != generations_double_small[0]; }));

    // With an error range
    CHECK_THROWS(ran5.GetDouble(100.9,1.0));

    // With negative numbers
    std::vector<double> generations_double_neg;
    for (int i = 0; i < 1000; ++i) {
        generations_double_neg.push_back(ran5.GetDouble(-100.9, 100.8));
    }
    REQUIRE(generations_double_neg.size() >= 2);
    CHECK(std::any_of(generations_double_neg.begin() + 1, generations_double_neg.end(), [&](const auto& x) { return x != generations_double_neg[0]; }));



    // Checking float generation
    // With default range
    std::vector<float> generations_float;
    for (int i = 0; i < 1000; ++i) {
        generations_float.push_back(ran5.GetFloat());
    }
    REQUIRE(generations_float.size() >= 2);
    CHECK(std::any_of(generations_float.begin() + 1, generations_float.end(), [&](const auto& x) { return x != generations_float[0]; }));
    
    // With large range
    std::vector<float> generations_float_large;
    for (int i = 0; i < 1000; ++i) {
        generations_float_large.push_back(ran5.GetFloat(0.01, 100000000.92));
    }
    REQUIRE(generations_float_large.size() >= 2);
    CHECK(std::any_of(generations_float_large.begin() + 1, generations_float_large.end(), [&](const auto& x) { return x != generations_float_large[0]; }));

    // With small range
    std::vector<float> generations_float_small;
    for (int i = 0; i < 1000; ++i) {
        generations_float_small.push_back(ran5.GetFloat(0.13, 0.89));
    }
    REQUIRE(generations_float_small.size() >= 2);
    CHECK(std::any_of(generations_float_small.begin() + 1, generations_float_small.end(), [&](const auto& x) { return x != generations_float_small[0]; }));

    // With an error range
    CHECK_THROWS(ran5.GetFloat(100.99,1.04));

    // With negative numbers
    std::vector<float> generations_float_neg;
    for (int i = 0; i < 1000; ++i) {
        generations_float_neg.push_back(ran5.GetFloat(-100.94, 100.86));
    }
    REQUIRE(generations_float_neg.size() >= 2);
    CHECK(std::any_of(generations_float_neg.begin() + 1, generations_float_neg.end(), [&](const auto& x) { return x != generations_float_neg[0]; }));



    // Checking char generation
    // With default range
    std::vector<char> generations_char;
    for (int i = 0; i < 1000; ++i) {
        generations_char.push_back(ran5.GetChar());
    }
    REQUIRE(generations_char.size() >= 2);
    CHECK(std::any_of(generations_char.begin() + 1, generations_char.end(), [&](const auto& x) { return x != generations_char[0]; }));

    // With custom range
    std::vector<char> generations_char_large;
    for (int i = 0; i < 1000; ++i) {
        generations_char_large.push_back(ran5.GetChar('B', 'G'));
    }
    REQUIRE(generations_char_large.size() >= 2);
    CHECK(std::any_of(generations_char_large.begin() + 1, generations_char_large.end(), [&](const auto& x) { return x != generations_char_large[0]; }));

    // With small range
    std::vector<char> generations_char_small;
    for (int i = 0; i < 1000; ++i) {
        generations_char_small.push_back(ran5.GetChar('A', 'B'));
    }
    REQUIRE(generations_char_small.size() >= 2);
    CHECK(std::any_of(generations_char_small.begin() + 1, generations_char_small.end(), [&](const auto& x) { return x != generations_char_small[0]; }));

    // With an error range
    CHECK_THROWS(ran5.GetChar('Z','A'));

    

    // Checking bool generation
    std::vector<bool> generations_bool;
    for (int i = 0; i < 1000; ++i) {
        generations_bool.push_back(ran5.GetBool());
    }
    REQUIRE(generations_bool.size() >= 2);
    CHECK(std::any_of(generations_bool.begin() + 1, generations_bool.end(), [&](const auto& x) { return x != generations_bool[0]; }));
    


    // Checking probability generation
    // With default range
    std::vector<bool> generations_p;
    for (int i = 0; i < 1000; ++i) {
        generations_p.push_back(ran5.P());
    }
    REQUIRE(generations_p.size() >= 2);
    CHECK(std::any_of(generations_p.begin() + 1, generations_p.end(), [&](const auto& x) { return x != generations_p[0]; }));

    // With small range
    std::vector<bool> generations_p_small;
    for (int i = 0; i < 1000; ++i) {
        generations_p_small.push_back(ran5.P(0.1));
    }
    REQUIRE(generations_p_small.size() >= 2);
    CHECK(std::any_of(generations_p_small.begin() + 1, generations_p_small.end(), [&](const auto& x) { return x != generations_p_small[0]; }));

    // With large range
    std::vector<bool> generations_p_large;
    for (int i = 0; i < 1000; ++i) {
        generations_p_large.push_back(ran5.P(0.9));
    }
    REQUIRE(generations_p_large.size() >= 2);
    CHECK(std::any_of(generations_p_large.begin() + 1, generations_p_large.end(), [&](const auto& x) { return x != generations_p_large[0]; }));

    // With small error range
    CHECK_THROWS(ran5.P(-1));

    // With large error range
    CHECK_THROWS(ran5.P(1.1));
}