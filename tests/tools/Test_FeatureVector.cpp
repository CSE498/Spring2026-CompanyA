
#include "../../source/tools/FeatureVector.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

using cse498::FeatureVector;

TEST_CASE("FeatureVector dot product", "[dot]") {
  FeatureVector<double> v1({1.0, 2.0, 3.0});

  FeatureVector<double> v2({4.0, 5.0, 6.0});

  FeatureVector<int> v3({1, 2, 3});
  FeatureVector<int> v4({4, 5, 6});

  REQUIRE(v1.dot(v2) == 32.0);
  REQUIRE(v3.dot(v4) == 32);
}

TEST_CASE("FeatureVector normalize", "[normalize]") {
  FeatureVector<double> v({3.0, 4.0});
  v.normalize();

  REQUIRE_THAT(v.at(0), Catch::Matchers::WithinAbs(0.6, 1e-9));

  REQUIRE_THAT(v.at(1), Catch::Matchers::WithinAbs(0.8, 1e-9));
}

TEST_CASE("FeatureVector at function", "[at]") {
  FeatureVector<double> v({5.0, 12.0, 9.0});

  REQUIRE(v.at(0) == 5.0);
  REQUIRE(v.at(1) == 12.0);
  REQUIRE(v.at(2) == 9.0);
}

TEST_CASE("FeatureVector at functions throws on out of range", "[at]") {
  FeatureVector<double> v({9.0, 5.0});
  REQUIRE_THROWS_AS(v.at(7), std::out_of_range);
}

TEST_CASE("FeatureVector dot product with itself", "[dot]") {
  FeatureVector<double> v({5.0, 12.0});
  REQUIRE(v.dot(v) == 169.0);
}

TEST_CASE("FeatureVector size function", "[size]") {
  FeatureVector<double> v({5.0, 12.0});
  REQUIRE(v.size() == 2);
}

TEST_CASE("FeatureVector pointer constructor", "[constructor]") {
  double data[] = {1.5, 2.5, 3.5};
  FeatureVector<double> v(data, 3);

  REQUIRE(v.size() == 3);
  REQUIRE(v.at(0) == 1.5);
  REQUIRE(v.at(1) == 2.5);
  REQUIRE(v.at(2) == 3.5);
}

TEST_CASE("FeatureVector pointer constructor with float", "[constructor]") {
  float data[] = {1.0f, 2.0f, 4.0f};
  FeatureVector<float> v(data, 3);

  REQUIRE(v.size() == 3);
  REQUIRE(v.at(0) == 1.0f);
  REQUIRE(v.at(1) == 2.0f);
  REQUIRE(v.at(2) == 4.0f);
}

TEST_CASE("FeatureVector with float type", "[float]") {
  FeatureVector<float> v1({1.0f, 2.0f, 3.0f});
  FeatureVector<float> v2({4.0f, 5.0f, 6.0f});

  REQUIRE(v1.dot(v2) == 32.0f);
}

TEST_CASE("FeatureVector normalize with float", "[float][normalize]") {
  FeatureVector<float> v({3.0f, 4.0f});
  v.normalize();

  REQUIRE_THAT(static_cast<double>(v.at(0)),
               Catch::Matchers::WithinAbs(0.6, 1e-5));
  REQUIRE_THAT(static_cast<double>(v.at(1)),
               Catch::Matchers::WithinAbs(0.8, 1e-5));
}

TEST_CASE("FeatureVector with int type", "[int]") {
  FeatureVector<int> v({10, 20, 30});

  REQUIRE(v.size() == 3);
  REQUIRE(v.at(0) == 10);
  REQUIRE(v.at(1) == 20);
  REQUIRE(v.at(2) == 30);
}

TEST_CASE("FeatureVector empty vector", "[empty]") {
  FeatureVector<double> v(std::vector<double>{});

  REQUIRE(v.size() == 0);
  REQUIRE_THROWS_AS(v.at(0), std::out_of_range);
}

TEST_CASE("FeatureVector normalize empty vector", "[empty][normalize]") {
  FeatureVector<double> v(std::vector<double>{});
  v.normalize();

  REQUIRE(v.size() == 0);
}

TEST_CASE("FeatureVector empty pointer constructor", "[empty][constructor]") {
  FeatureVector<double> v(static_cast<const double *>(nullptr), 0);

  REQUIRE(v.size() == 0);
  REQUIRE_THROWS_AS(v.at(0), std::out_of_range);
}

TEST_CASE("FeatureVector scale normalizes then scales", "[scale]") {
  FeatureVector<double> v({3.0, 4.0});
  v.scale(10.0);

  REQUIRE_THAT(v.at(0), Catch::Matchers::WithinAbs(6.0, 1e-9));
  REQUIRE_THAT(v.at(1), Catch::Matchers::WithinAbs(8.0, 1e-9));
}

TEST_CASE("FeatureVector scale returns reference for chaining", "[scale]") {
  FeatureVector<double> v({3.0, 4.0});
  FeatureVector<double> &ref = v.scale(5.0);

  REQUIRE(&ref == &v);
}

TEST_CASE("FeatureVector scale with zero vector", "[scale][empty]") {
  FeatureVector<double> v({0.0, 0.0});
  v.scale(10.0);

  REQUIRE(v.at(0) == 0.0);
  REQUIRE(v.at(1) == 0.0);
}

TEST_CASE("FeatureVector rotate function", "[rotate]") {
  FeatureVector<double> v({3.0, 4.0});
  v.rotate(0, 1, M_PI / 2);
  REQUIRE_THAT(v.at(0), Catch::Matchers::WithinAbs(-4.0, 1e-9));
  REQUIRE_THAT(v.at(1), Catch::Matchers::WithinAbs(3.0, 1e-9));
}

// TEST_CASE("FeatureVector add function", "[add]")
// {
//     FeatureVector<double> v1({1.0, 2.0});
//     FeatureVector<double> v2({3.0, 4.0});
//     FeatureVector<double> v = v1 + v2;

//     REQUIRE(v.at(0) == 4.0);
//     REQUIRE(v.at(1) == 6.0);
// }

// TEST_CASE("FeatureVector subtract function", "[subtract]")
// {
//     FeatureVector<double> v1({1.0, 2.0});
//     FeatureVector<double> v2({3.0, 4.0});
//     FeatureVector<double> v = v1 - v2;

//     REQUIRE(v.at(0) == -2.0);
//     REQUIRE(v.at(1) == -2.0);
// }

TEST_CASE("FeatureVector hadamard product", "[hadamard]") {
  FeatureVector<double> v1({2.0, 3.0, 4.0});
  FeatureVector<double> v2({5.0, 6.0, 7.0});
  FeatureVector<double> result = v1.hadamard(v2);

  REQUIRE(result.at(0) == 10.0);
  REQUIRE(result.at(1) == 18.0);
  REQUIRE(result.at(2) == 28.0);
}

TEST_CASE("FeatureVector hadamard with zeros", "[hadamard]") {
  FeatureVector<double> v1({1.0, 2.0, 3.0});
  FeatureVector<double> v2({0.0, 0.0, 0.0});
  FeatureVector<double> result = v1.hadamard(v2);

  REQUIRE(result.at(0) == 0.0);
  REQUIRE(result.at(1) == 0.0);
  REQUIRE(result.at(2) == 0.0);
}

TEST_CASE("FeatureVector hadamard mismatched sizes throws", "[hadamard]") {
  FeatureVector<double> v1({1.0, 2.0});
  FeatureVector<double> v2({1.0, 2.0, 3.0});

  REQUIRE_THROWS_AS(v1.hadamard(v2), std::invalid_argument);
}

TEST_CASE("FeatureVector hadamard with int type", "[hadamard][int]") {
  FeatureVector<int> v1({2, 3, 4});
  FeatureVector<int> v2({5, 6, 7});
  FeatureVector<int> result = v1.hadamard(v2);

  REQUIRE(result.at(0) == 10);
  REQUIRE(result.at(1) == 18);
  REQUIRE(result.at(2) == 28);
}