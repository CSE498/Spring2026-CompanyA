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


TEST_CASE("Test Random Copy Operator", "[core]") {
    cse498::Random generated;
    cse498::Random generated_copy(generated);
    REQUIRE(generated_copy.GetSeed() == generated.GetSeed());

    cse498::Random set;
    set.SetSeed(1000);
    cse498::Random set_copy(set);
    REQUIRE(set_copy.GetSeed() == 1000);
}

/*
TEST_CASE("Test Random Assignment Operator", "[core]") {
    cse498::Random generated_one;
    cse498::Random generated_two;
    generated_two = generated_one;
    REQUIRE(generated_two.GetSeed() == generated_one.GetSeed());

    cse498::Random set_one;
    set_one.SetSeed(1000);
    cse498::Random set_two;
    set_two = set_one;
    REQUIRE(set_two.GetSeed() == set_one.GetSeed());

    cse498::Random generated_three;
    generated_three = generated_three;
    REQUIRE(generated_three.GetSeed() == generated_three.GetSeed());

    cse498::Random set_three;
    set_three.SetSeed(1000);
    set_three = set_three;
    REQUIRE(set_three.GetSeed() == set_three.GetSeed());
}
*/

TEST_CASE("Test Random Move Operator", "[core]") {
    cse498::Random generated;
    cse498::Random generated_moved(std::move(generated));
    REQUIRE(generated_moved.GetSeed() == generated.GetSeed());

    cse498::Random set;
    set.SetSeed(1000);
    cse498::Random set_moved(std::move(set));
    REQUIRE(set_moved.GetSeed() == 1000);
}

TEST_CASE("Test Seed Setting", "[core]") {
    cse498::Random ran1;
    cse498::Random ran2;
    uint64_t generatedSeed = ran2.GetSeed();
    ran2.SetSeed(1000);   // Manually Set the seed
    CHECK(ran2.GetSeed() != generatedSeed);
    CHECK(ran2.GetSeed() == 1000);
}

TEST_CASE("Test Random Integer Generation with Set Seed", "[core]") {
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

    // With an error range (ill-formed parameter test cases)
    CHECK_THROWS(ran4.GetInt(100,1));

    // With negative numbers
    std::vector<int> generations_int_neg;
    for (int i = 0; i < 1000; ++i) {
        generations_int_neg.push_back(ran4.GetInt(-100, 100));
    }
    REQUIRE(generations_int_neg.size() >= 2);
    CHECK(std::any_of(generations_int_neg.begin() + 1, generations_int_neg.end(), [&](const auto& x) { return x != generations_int_neg[0]; }));
}

TEST_CASE("Test Random Double Generation with Set Seed", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    cse498::Random ran5;
    ran5.SetSeed(1000);

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

    // With an error range (ill-formed parameter test cases)
    CHECK_THROWS(ran5.GetDouble(100.9,1.0));

    // With negative numbers
    std::vector<double> generations_double_neg;
    for (int i = 0; i < 1000; ++i) {
        generations_double_neg.push_back(ran5.GetDouble(-100.9, 100.8));
    }
    REQUIRE(generations_double_neg.size() >= 2);
    CHECK(std::any_of(generations_double_neg.begin() + 1, generations_double_neg.end(), [&](const auto& x) { return x != generations_double_neg[0]; }));
}

TEST_CASE("Test Random Float Generation with Set Seed", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    cse498::Random ran6;
    ran6.SetSeed(1000);

    // Checking float generation
    // With default range
    std::vector<float> generations_float;
    for (int i = 0; i < 1000; ++i) {
        generations_float.push_back(ran6.GetFloat());
    }
    REQUIRE(generations_float.size() >= 2);
    CHECK(std::any_of(generations_float.begin() + 1, generations_float.end(), [&](const auto& x) { return x != generations_float[0]; }));
    
    // With large range
    std::vector<float> generations_float_large;
    for (int i = 0; i < 1000; ++i) {
        generations_float_large.push_back(ran6.GetFloat(0.01, 100000000.92));
    }
    REQUIRE(generations_float_large.size() >= 2);
    CHECK(std::any_of(generations_float_large.begin() + 1, generations_float_large.end(), [&](const auto& x) { return x != generations_float_large[0]; }));

    // With small range
    std::vector<float> generations_float_small;
    for (int i = 0; i < 1000; ++i) {
        generations_float_small.push_back(ran6.GetFloat(0.13, 0.89));
    }
    REQUIRE(generations_float_small.size() >= 2);
    CHECK(std::any_of(generations_float_small.begin() + 1, generations_float_small.end(), [&](const auto& x) { return x != generations_float_small[0]; }));

    // With an error range (ill-formed parameter test cases)
    CHECK_THROWS(ran6.GetFloat(100.99,1.04));

    // With negative numbers
    std::vector<float> generations_float_neg;
    for (int i = 0; i < 1000; ++i) {
        generations_float_neg.push_back(ran6.GetFloat(-100.94, 100.86));
    }
    REQUIRE(generations_float_neg.size() >= 2);
    CHECK(std::any_of(generations_float_neg.begin() + 1, generations_float_neg.end(), [&](const auto& x) { return x != generations_float_neg[0]; }));
}

TEST_CASE("Test Random Char Generation with Set Seed", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    cse498::Random ran7;
    ran7.SetSeed(1000);

    // Checking char generation
    // With default range
    std::vector<char> generations_char;
    for (int i = 0; i < 1000; ++i) {
        generations_char.push_back(ran7.GetChar());
    }
    REQUIRE(generations_char.size() >= 2);
    CHECK(std::any_of(generations_char.begin() + 1, generations_char.end(), [&](const auto& x) { return x != generations_char[0]; }));

    // With custom range
    std::vector<char> generations_char_large;
    for (int i = 0; i < 1000; ++i) {
        generations_char_large.push_back(ran7.GetChar('B', 'G'));
    }
    REQUIRE(generations_char_large.size() >= 2);
    CHECK(std::any_of(generations_char_large.begin() + 1, generations_char_large.end(), [&](const auto& x) { return x != generations_char_large[0]; }));

    // With small range
    std::vector<char> generations_char_small;
    for (int i = 0; i < 1000; ++i) {
        generations_char_small.push_back(ran7.GetChar('A', 'B'));
    }
    REQUIRE(generations_char_small.size() >= 2);
    CHECK(std::any_of(generations_char_small.begin() + 1, generations_char_small.end(), [&](const auto& x) { return x != generations_char_small[0]; }));

    // With an error range (ill-formed parameter test cases)
    CHECK_THROWS(ran7.GetChar('Z','A'));
}

TEST_CASE("Test Random Boolean Generation with Set Seed", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    cse498::Random ran8;
    ran8.SetSeed(1000);

    // Checking bool generation
    std::vector<bool> generations_bool;
    for (int i = 0; i < 1000; ++i) {
        generations_bool.push_back(ran8.GetBool());
    }
    REQUIRE(generations_bool.size() >= 2);
    CHECK(std::any_of(generations_bool.begin() + 1, generations_bool.end(), [&](const auto& x) { return x != generations_bool[0]; }));
}

TEST_CASE("Test Random Proability Generation with Set Seed", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    cse498::Random ran9;
    ran9.SetSeed(1000);

    // Checking probability generation
    // With default range
    std::vector<bool> generations_p;
    for (int i = 0; i < 1000; ++i) {
        generations_p.push_back(ran9.P());
    }
    REQUIRE(generations_p.size() >= 2);
    CHECK(std::any_of(generations_p.begin() + 1, generations_p.end(), [&](const auto& x) { return x != generations_p[0]; }));

    // With small range
    std::vector<bool> generations_p_small;
    for (int i = 0; i < 1000; ++i) {
        generations_p_small.push_back(ran9.P(0.1));
    }
    REQUIRE(generations_p_small.size() >= 2);
    CHECK(std::any_of(generations_p_small.begin() + 1, generations_p_small.end(), [&](const auto& x) { return x != generations_p_small[0]; }));

    // With large range
    std::vector<bool> generations_p_large;
    for (int i = 0; i < 1000; ++i) {
        generations_p_large.push_back(ran9.P(0.9));
    }
    REQUIRE(generations_p_large.size() >= 2);
    CHECK(std::any_of(generations_p_large.begin() + 1, generations_p_large.end(), [&](const auto& x) { return x != generations_p_large[0]; }));

    // With small error range (ill-formed parameter test cases)
    CHECK_THROWS(ran9.P(-1));

    // With large error range (ill-formed parameter test cases)
    CHECK_THROWS(ran9.P(1.1));
}

TEST_CASE("Test Random Integer Generation with Generated Seed", "[core]") {
    //// Check that it is randomly generating numbers with a generated seed
    cse498::Random ran10;

    /// Checking int generation
    // With default range
    std::vector<int> generations_int;
    for (int i = 0; i < 1000; ++i) {
        generations_int.push_back(ran10.GetInt());
    }
    REQUIRE(generations_int.size() >= 2);
    CHECK(std::any_of(generations_int.begin() + 1, generations_int.end(), [&](const auto& x) { return x != generations_int[0]; }));

    // With large range
    std::vector<int> generations_int_large;
    for (int i = 0; i < 1000; ++i) {
        generations_int_large.push_back(ran10.GetInt(0, 100000000));
    }
    REQUIRE(generations_int_large.size() >= 2);
    CHECK(std::any_of(generations_int_large.begin() + 1, generations_int_large.end(), [&](const auto& x) { return x != generations_int_large[0]; }));

    // With small range
    std::vector<int> generations_int_small;
    for (int i = 0; i < 1000; ++i) {
        generations_int_small.push_back(ran10.GetInt(1, 3));
    }
    REQUIRE(generations_int_small.size() >= 2);
    CHECK(std::any_of(generations_int_small.begin() + 1, generations_int_small.end(), [&](const auto& x) { return x != generations_int_small[0]; }));

    // With an error range (ill-formed parameter test cases)
    CHECK_THROWS(ran10.GetInt(100,1));

    // With negative numbers
    std::vector<int> generations_int_neg;
    for (int i = 0; i < 1000; ++i) {
        generations_int_neg.push_back(ran10.GetInt(-100, 100));
    }
    REQUIRE(generations_int_neg.size() >= 2);
    CHECK(std::any_of(generations_int_neg.begin() + 1, generations_int_neg.end(), [&](const auto& x) { return x != generations_int_neg[0]; }));
}

TEST_CASE("Test Random Double Generation with Generated Seed", "[core]") {
    //// Check that it is randomly generating numbers with a generated seed
    cse498::Random ran11;

    /// Checking double generation
    // With default range
    std::vector<double> generations_double;
    for (int i = 0; i < 1000; ++i) {
        generations_double.push_back(ran11.GetDouble());
    }
    REQUIRE(generations_double.size() >= 2);
    CHECK(std::any_of(generations_double.begin() + 1, generations_double.end(), [&](const auto& x) { return x != generations_double[0]; }));

    // With large range
    std::vector<double> generations_double_large;
    for (int i = 0; i < 1000; ++i) {
        generations_double_large.push_back(ran11.GetDouble(0.0, 100000000.9));
    }
    REQUIRE(generations_double_large.size() >= 2);
    CHECK(std::any_of(generations_double_large.begin() + 1, generations_double_large.end(), [&](const auto& x) { return x != generations_double_large[0]; }));

    // With small range
    std::vector<double> generations_double_small;
    for (int i = 0; i < 1000; ++i) {
        generations_double_small.push_back(ran11.GetDouble(0.1, 0.8));
    }
    REQUIRE(generations_double_small.size() >= 2);
    CHECK(std::any_of(generations_double_small.begin() + 1, generations_double_small.end(), [&](const auto& x) { return x != generations_double_small[0]; }));

    // With an error range (ill-formed parameter test cases)
    CHECK_THROWS(ran11.GetDouble(100.9,1.0));

    // With negative numbers
    std::vector<double> generations_double_neg;
    for (int i = 0; i < 1000; ++i) {
        generations_double_neg.push_back(ran11.GetDouble(-100.9, 100.8));
    }
    REQUIRE(generations_double_neg.size() >= 2);
    CHECK(std::any_of(generations_double_neg.begin() + 1, generations_double_neg.end(), [&](const auto& x) { return x != generations_double_neg[0]; }));
}

TEST_CASE("Test Random Float Generation with Generated Seed", "[core]") {
    //// Check that it is randomly generating numbers with a generated seed
    cse498::Random ran12;

    /// Checking float generation
    // With default range
    std::vector<float> generations_float;
    for (int i = 0; i < 1000; ++i) {
        generations_float.push_back(ran12.GetFloat());
    }
    REQUIRE(generations_float.size() >= 2);
    CHECK(std::any_of(generations_float.begin() + 1, generations_float.end(), [&](const auto& x) { return x != generations_float[0]; }));
    
    // With large range
    std::vector<float> generations_float_large;
    for (int i = 0; i < 1000; ++i) {
        generations_float_large.push_back(ran12.GetFloat(0.01, 100000000.92));
    }
    REQUIRE(generations_float_large.size() >= 2);
    CHECK(std::any_of(generations_float_large.begin() + 1, generations_float_large.end(), [&](const auto& x) { return x != generations_float_large[0]; }));

    // With small range
    std::vector<float> generations_float_small;
    for (int i = 0; i < 1000; ++i) {
        generations_float_small.push_back(ran12.GetFloat(0.13, 0.89));
    }
    REQUIRE(generations_float_small.size() >= 2);
    CHECK(std::any_of(generations_float_small.begin() + 1, generations_float_small.end(), [&](const auto& x) { return x != generations_float_small[0]; }));

    // With an error range (ill-formed parameter test cases)
    CHECK_THROWS(ran12.GetFloat(100.99,1.04));

    // With negative numbers
    std::vector<float> generations_float_neg;
    for (int i = 0; i < 1000; ++i) {
        generations_float_neg.push_back(ran12.GetFloat(-100.94, 100.86));
    }
    REQUIRE(generations_float_neg.size() >= 2);
    CHECK(std::any_of(generations_float_neg.begin() + 1, generations_float_neg.end(), [&](const auto& x) { return x != generations_float_neg[0]; }));
}

TEST_CASE("Test Random Char Generation with Generated Seed", "[core]") {
    //// Check that it is randomly generating numbers with a generated seed
    cse498::Random ran13;

    // Checking char generation
    // With default range
    std::vector<char> generations_char;
    for (int i = 0; i < 1000; ++i) {
        generations_char.push_back(ran13.GetChar());
    }
    REQUIRE(generations_char.size() >= 2);
    CHECK(std::any_of(generations_char.begin() + 1, generations_char.end(), [&](const auto& x) { return x != generations_char[0]; }));

    // With custom range
    std::vector<char> generations_char_large;
    for (int i = 0; i < 1000; ++i) {
        generations_char_large.push_back(ran13.GetChar('B', 'G'));
    }
    REQUIRE(generations_char_large.size() >= 2);
    CHECK(std::any_of(generations_char_large.begin() + 1, generations_char_large.end(), [&](const auto& x) { return x != generations_char_large[0]; }));

    // With small range
    std::vector<char> generations_char_small;
    for (int i = 0; i < 1000; ++i) {
        generations_char_small.push_back(ran13.GetChar('A', 'B'));
    }
    REQUIRE(generations_char_small.size() >= 2);
    CHECK(std::any_of(generations_char_small.begin() + 1, generations_char_small.end(), [&](const auto& x) { return x != generations_char_small[0]; }));

    // With an error range (ill-formed parameter test cases)
    CHECK_THROWS(ran13.GetChar('Z','A'));
}

TEST_CASE("Test Random Boolean Generation with Generated Seed", "[core]") {
    //// Check that it is randomly generating numbers with a generated seed
    cse498::Random ran13;

    /// Checking bool generation
    std::vector<bool> generations_bool;
    for (int i = 0; i < 1000; ++i) {
        generations_bool.push_back(ran13.GetBool());
    }
    REQUIRE(generations_bool.size() >= 2);
    CHECK(std::any_of(generations_bool.begin() + 1, generations_bool.end(), [&](const auto& x) { return x != generations_bool[0]; }));
}

TEST_CASE("Test Random Probability Generation with Generated Seed", "[core]") {
    //// Check that it is randomly generating numbers with a generated seed
    cse498::Random ran14;

    /// Checking probability generation
    // With default range
    std::vector<bool> generations_p;
    for (int i = 0; i < 1000; ++i) {
        generations_p.push_back(ran14.P());
    }
    REQUIRE(generations_p.size() >= 2);
    CHECK(std::any_of(generations_p.begin() + 1, generations_p.end(), [&](const auto& x) { return x != generations_p[0]; }));

    // With small range
    std::vector<bool> generations_p_small;
    for (int i = 0; i < 1000; ++i) {
        generations_p_small.push_back(ran14.P(0.1));
    }
    REQUIRE(generations_p_small.size() >= 2);
    CHECK(std::any_of(generations_p_small.begin() + 1, generations_p_small.end(), [&](const auto& x) { return x != generations_p_small[0]; }));

    // With large range
    std::vector<bool> generations_p_large;
    for (int i = 0; i < 1000; ++i) {
        generations_p_large.push_back(ran14.P(0.9));
    }
    REQUIRE(generations_p_large.size() >= 2);
    CHECK(std::any_of(generations_p_large.begin() + 1, generations_p_large.end(), [&](const auto& x) { return x != generations_p_large[0]; }));

    // With small error range (ill-formed parameter test cases)
    CHECK_THROWS(ran14.P(-1));

    // With large error range (ill-formed parameter test cases)
    CHECK_THROWS(ran14.P(1.1));
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

TEST_CASE("Test Random Double Generation Reproducability with Same Seed", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    cse498::Random ran17;
    ran17.SetSeed(1000);
    cse498::Random ran18;
    ran17.SetSeed(1000);

    /// Checking double generation
    // With default range
    std::vector<double> generations_double_1;
    std::vector<double> generations_double_2;
    for (int i = 0; i < 1000; ++i) {
        generations_double_1.push_back(ran17.GetDouble());
        generations_double_2.push_back(ran18.GetDouble());
    }
    REQUIRE(generations_double_1.size() == generations_double_2.size());

    // With large range
    std::vector<double> generations_double_large_1;
    std::vector<double> generations_double_large_2;
    for (int i = 0; i < 1000; ++i) {
        generations_double_large_1.push_back(ran17.GetDouble(0.0, 100000000.9));
        generations_double_large_2.push_back(ran18.GetDouble(0.0, 100000000.9));
    }
    REQUIRE(generations_double_large_1.size() == generations_double_large_2.size());
    
    // With small range
    std::vector<double> generations_double_small_1;
    std::vector<double> generations_double_small_2;
    for (int i = 0; i < 1000; ++i) {
        generations_double_small_1.push_back(ran17.GetDouble(0.1, 0.8));
        generations_double_small_1.push_back(ran18.GetDouble(0.1, 0.8));
    }

    // With negative numbers
    std::vector<double> generations_double_neg_1;
    std::vector<double> generations_double_neg_2;
    for (int i = 0; i < 1000; ++i) {
        generations_double_neg_1.push_back(ran17.GetDouble(-100.9, 100.8));
        generations_double_neg_2.push_back(ran18.GetDouble(-100.9, 100.8));
    }
    REQUIRE(generations_double_neg_1.size() == generations_double_neg_2.size());
}

TEST_CASE("Test Random Float Generation Reproducability with Same Seed", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    cse498::Random ran19;
    ran19.SetSeed(1000);
    cse498::Random ran20;
    ran20.SetSeed(1000);

    // Checking float generation
    // With default range
    std::vector<float> generations_float_1;
    std::vector<float> generations_float_2;
    for (int i = 0; i < 1000; ++i) {
        generations_float_1.push_back(ran19.GetFloat());
        generations_float_2.push_back(ran20.GetFloat());
    }
    REQUIRE(generations_float_1.size() == generations_float_2.size());
    CHECK(generations_float_1 == generations_float_2);
    
    // With large range
    std::vector<float> generations_float_large_1;
    std::vector<float> generations_float_large_2;
    for (int i = 0; i < 1000; ++i) {
        generations_float_large_1.push_back(ran19.GetFloat(0.01, 100000000.92));
        generations_float_large_2.push_back(ran20.GetFloat(0.01, 100000000.92));
    }
    REQUIRE(generations_float_large_1.size() == generations_float_large_2.size());
    CHECK(generations_float_large_1 == generations_float_large_2);

    // With small range
    std::vector<float> generations_float_small_1;
    std::vector<float> generations_float_small_2;
    for (int i = 0; i < 1000; ++i) {
        generations_float_small_1.push_back(ran19.GetFloat(0.13, 0.89));
        generations_float_small_2.push_back(ran20.GetFloat(0.13, 0.89));
    }
    REQUIRE(generations_float_small_1.size() == generations_float_small_2.size());
    CHECK(generations_float_small_1 == generations_float_small_2);

    // With negative numbers
    std::vector<float> generations_float_neg_1;
    std::vector<float> generations_float_neg_2;
    for (int i = 0; i < 1000; ++i) {
        generations_float_neg_1.push_back(ran19.GetFloat(-100.94, 100.86));
        generations_float_neg_2.push_back(ran20.GetFloat(-100.94, 100.86));
    }
    REQUIRE(generations_float_neg_1.size() == generations_float_neg_2.size());
    CHECK(generations_float_neg_1 == generations_float_neg_2);
}

TEST_CASE("Test Random Char Generation Reproducability with Same Seed", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    cse498::Random ran21;
    ran21.SetSeed(1000);
    cse498::Random ran22;
    ran22.SetSeed(1000);

    // Checking char generation
    // With default range
    std::vector<char> generations_char_1;
    std::vector<char> generations_char_2;
    for (int i = 0; i < 1000; ++i) {
        generations_char_1.push_back(ran21.GetChar());
        generations_char_2.push_back(ran22.GetChar());
    }
    REQUIRE(generations_char_1.size() == generations_char_2.size());
    CHECK(generations_char_1 == generations_char_2);

    // With custom range
    std::vector<char> generations_char_large_1;
    std::vector<char> generations_char_large_2;
    for (int i = 0; i < 1000; ++i) {
        generations_char_large_1.push_back(ran21.GetChar('B', 'G'));
        generations_char_large_2.push_back(ran22.GetChar('B', 'G'));
    }
    REQUIRE(generations_char_large_1.size() == generations_char_large_2.size());
    CHECK(generations_char_large_1 == generations_char_large_2);

    // With small range
    std::vector<char> generations_char_small_1;
    std::vector<char> generations_char_small_2;
    for (int i = 0; i < 1000; ++i) {
        generations_char_small_1.push_back(ran21.GetChar('A', 'B'));
        generations_char_small_2.push_back(ran22.GetChar('A', 'B'));
    }
    REQUIRE(generations_char_small_1.size() == generations_char_small_2.size());
    CHECK(generations_char_small_1 == generations_char_small_2);
}

TEST_CASE("Test Random Boolean Generation Reproducability with Same Seed", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    cse498::Random ran23;
    ran23.SetSeed(1000);
    cse498::Random ran24;
    ran24.SetSeed(1000);

    // Checking bool generation
    std::vector<bool> generations_bool_1;
    std::vector<bool> generations_bool_2;
    for (int i = 0; i < 1000; ++i) {
        generations_bool_1.push_back(ran23.GetBool());
        generations_bool_2.push_back(ran24.GetBool());
    }
    REQUIRE(generations_bool_1.size() == generations_bool_2.size());
    CHECK(generations_bool_1 == generations_bool_2);
}

TEST_CASE("Test Random Proability Generation Reproducability with Same Seed", "[core]") {
    //// Check that it is randomly generating numbers with a set seed
    cse498::Random ran25;
    ran25.SetSeed(1000);
    cse498::Random ran26;
    ran26.SetSeed(1000);

    // Checking probability generation
    // With default range
    std::vector<bool> generations_p_1;
    std::vector<bool> generations_p_2;
    for (int i = 0; i < 1000; ++i) {
        generations_p_1.push_back(ran25.P());
        generations_p_2.push_back(ran26.P());
    }
    REQUIRE(generations_p_1.size() == generations_p_2.size());
    CHECK(generations_p_1 == generations_p_2);

    // With small range
    std::vector<bool> generations_p_small_1;
    std::vector<bool> generations_p_small_2;
    for (int i = 0; i < 1000; ++i) {
        generations_p_small_1.push_back(ran25.P(0.1));
        generations_p_small_2.push_back(ran26.P(0.1));
    }
    REQUIRE(generations_p_small_1.size() == generations_p_small_2.size());
    CHECK(generations_p_small_1 == generations_p_small_2);

    // With large range
    std::vector<bool> generations_p_large_1;
    std::vector<bool> generations_p_large_2;
    for (int i = 0; i < 1000; ++i) {
        generations_p_large_1.push_back(ran25.P(0.9));
        generations_p_large_2.push_back(ran26.P(0.9));
    }
    REQUIRE(generations_p_large_1.size() == generations_p_large_2.size());
    CHECK(generations_p_large_1 == generations_p_large_2);
}