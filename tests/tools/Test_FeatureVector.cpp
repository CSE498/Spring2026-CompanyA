
#include "../../source/tools/FeatureVector.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include <cmath>
#include <stdexcept>

namespace {

enum class TestFeature {
  Health = 0,
  Energy,
  DistanceToGoal,
  NearbyEnemies,
  COUNT
};

using TestVector = cse498::FeatureVector<TestFeature>;

} // namespace

TEST_CASE("FeatureVector default constructor initializes all values to zero",
          "[FeatureVector]") {
  TestVector fv;

  REQUIRE(fv.size() == 4);
  REQUIRE(fv.get(TestFeature::Health) == 0.0);
  REQUIRE(fv.get(TestFeature::Energy) == 0.0);
  REQUIRE(fv.get(TestFeature::DistanceToGoal) == 0.0);
  REQUIRE(fv.get(TestFeature::NearbyEnemies) == 0.0);
}

TEST_CASE("FeatureVector initializer list constructor stores values correctly",
          "[FeatureVector]") {
  TestVector fv{10.0, 20.0, 30.0, 40.0};

  REQUIRE(fv.size() == 4);
  REQUIRE(fv.get(TestFeature::Health) == 10.0);
  REQUIRE(fv.get(TestFeature::Energy) == 20.0);
  REQUIRE(fv.get(TestFeature::DistanceToGoal) == 30.0);
  REQUIRE(fv.get(TestFeature::NearbyEnemies) == 40.0);
}

TEST_CASE("FeatureVector set and get work correctly", "[FeatureVector]") {
  TestVector fv;

  fv.set(TestFeature::Health, 99.0);
  fv.set(TestFeature::Energy, 55.5);

  REQUIRE(fv.get(TestFeature::Health) == 99.0);
  REQUIRE(fv.get(TestFeature::Energy) == 55.5);
  REQUIRE(fv.get(TestFeature::DistanceToGoal) == 0.0);
}

TEST_CASE("FeatureVector at returns values by index", "[FeatureVector]") {
  TestVector fv{1.0, 2.0, 3.0, 4.0};

  REQUIRE(fv.at(0) == 1.0);
  REQUIRE(fv.at(1) == 2.0);
  REQUIRE(fv.at(2) == 3.0);
  REQUIRE(fv.at(3) == 4.0);
}

TEST_CASE("FeatureVector equality and inequality operators work",
          "[FeatureVector]") {
  TestVector a{1.0, 2.0, 3.0, 4.0};
  TestVector b{1.0, 2.0, 3.0, 4.0};
  TestVector c{4.0, 3.0, 2.0, 1.0};

  REQUIRE(a == b);
  REQUIRE(a != c);
}

TEST_CASE("FeatureVector addition works correctly", "[FeatureVector]") {
  TestVector a{1.0, 2.0, 3.0, 4.0};
  TestVector b{10.0, 20.0, 30.0, 40.0};

  TestVector result = a + b;

  REQUIRE(result.get(TestFeature::Health) == 11.0);
  REQUIRE(result.get(TestFeature::Energy) == 22.0);
  REQUIRE(result.get(TestFeature::DistanceToGoal) == 33.0);
  REQUIRE(result.get(TestFeature::NearbyEnemies) == 44.0);
}

TEST_CASE("FeatureVector subtraction works correctly", "[FeatureVector]") {
  TestVector a{10.0, 20.0, 30.0, 40.0};
  TestVector b{1.0, 2.0, 3.0, 4.0};

  TestVector result = a - b;

  REQUIRE(result.get(TestFeature::Health) == 9.0);
  REQUIRE(result.get(TestFeature::Energy) == 18.0);
  REQUIRE(result.get(TestFeature::DistanceToGoal) == 27.0);
  REQUIRE(result.get(TestFeature::NearbyEnemies) == 36.0);
}

TEST_CASE("FeatureVector scalar multiplication works correctly",
          "[FeatureVector]") {
  TestVector a{1.0, 2.0, 3.0, 4.0};

  TestVector result1 = a * 2.0;
  TestVector result2 = 3.0 * a;

  REQUIRE(result1.get(TestFeature::Health) == 2.0);
  REQUIRE(result1.get(TestFeature::Energy) == 4.0);
  REQUIRE(result1.get(TestFeature::DistanceToGoal) == 6.0);
  REQUIRE(result1.get(TestFeature::NearbyEnemies) == 8.0);

  REQUIRE(result2.get(TestFeature::Health) == 3.0);
  REQUIRE(result2.get(TestFeature::Energy) == 6.0);
  REQUIRE(result2.get(TestFeature::DistanceToGoal) == 9.0);
  REQUIRE(result2.get(TestFeature::NearbyEnemies) == 12.0);
}

TEST_CASE("FeatureVector compound assignment operators work correctly",
          "[FeatureVector]") {
  TestVector a{1.0, 2.0, 3.0, 4.0};
  TestVector b{10.0, 20.0, 30.0, 40.0};

  SECTION("operator+=") {
    a += b;
    REQUIRE(a.get(TestFeature::Health) == 11.0);
    REQUIRE(a.get(TestFeature::Energy) == 22.0);
    REQUIRE(a.get(TestFeature::DistanceToGoal) == 33.0);
    REQUIRE(a.get(TestFeature::NearbyEnemies) == 44.0);
  }

  SECTION("operator-=") {
    b -= a;
    REQUIRE(b.get(TestFeature::Health) == 9.0);
    REQUIRE(b.get(TestFeature::Energy) == 18.0);
    REQUIRE(b.get(TestFeature::DistanceToGoal) == 27.0);
    REQUIRE(b.get(TestFeature::NearbyEnemies) == 36.0);
  }

  SECTION("operator*=") {
    a *= 2.0;
    REQUIRE(a.get(TestFeature::Health) == 2.0);
    REQUIRE(a.get(TestFeature::Energy) == 4.0);
    REQUIRE(a.get(TestFeature::DistanceToGoal) == 6.0);
    REQUIRE(a.get(TestFeature::NearbyEnemies) == 8.0);
  }
}

TEST_CASE("FeatureVector dot product works correctly", "[FeatureVector]") {
  TestVector a{1.0, 2.0, 3.0, 4.0};
  TestVector b{5.0, 6.0, 7.0, 8.0};

  // 1*5 + 2*6 + 3*7 + 4*8 = 70
  REQUIRE(a.dot(b) == 70.0);
}

TEST_CASE("FeatureVector sum works correctly", "[FeatureVector]") {
  TestVector fv{1.5, 2.5, 3.0, 4.0};

  REQUIRE(fv.sum() == 11.0);
}

TEST_CASE("FeatureVector hadamard product works correctly", "[FeatureVector]") {
  TestVector a{1.0, 2.0, 3.0, 4.0};
  TestVector b{2.0, 3.0, 4.0, 5.0};

  TestVector result = a.hadamard(b);

  REQUIRE(result.get(TestFeature::Health) == 2.0);
  REQUIRE(result.get(TestFeature::Energy) == 6.0);
  REQUIRE(result.get(TestFeature::DistanceToGoal) == 12.0);
  REQUIRE(result.get(TestFeature::NearbyEnemies) == 20.0);
}

TEST_CASE("FeatureVector normalize scales vector to unit length",
          "[FeatureVector]") {
  TestVector fv{3.0, 4.0, 0.0, 0.0};

  fv.normalize();

  REQUIRE(fv.get(TestFeature::Health) == Approx(0.6));
  REQUIRE(fv.get(TestFeature::Energy) == Approx(0.8));
  REQUIRE(fv.get(TestFeature::DistanceToGoal) == Approx(0.0));
  REQUIRE(fv.get(TestFeature::NearbyEnemies) == Approx(0.0));

  REQUIRE(fv.dot(fv) == Approx(1.0));
}

TEST_CASE("FeatureVector normalize leaves zero vector unchanged",
          "[FeatureVector]") {
  TestVector fv{0.0, 0.0, 0.0, 0.0};

  fv.normalize();

  REQUIRE(fv.get(TestFeature::Health) == 0.0);
  REQUIRE(fv.get(TestFeature::Energy) == 0.0);
  REQUIRE(fv.get(TestFeature::DistanceToGoal) == 0.0);
  REQUIRE(fv.get(TestFeature::NearbyEnemies) == 0.0);
}