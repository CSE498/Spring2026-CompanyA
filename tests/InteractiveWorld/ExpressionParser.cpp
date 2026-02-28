#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include <cassert>
#include <cmath>
#include <stdexcept>

// GPT Generated to diagnose testcase not running locally. (lines 8-13)
// Keep error-path tests portable: convert assert failures into exceptions here.
#undef assert
#define assert(condition)                                                      \
  do {                                                                         \
    if (!(condition)) {                                                        \
      throw std::runtime_error("ExpressionParser assertion hit");              \
    }                                                                          \
  } while (false)

#include "../../source/tools/InteractiveWorld/ExpressionParser.hpp"

// used:
//  - https://github.com/catchorg/Catch2 for Catch2 implementation reference
//  - Codex for test case generation ideas

using namespace cse498;
TEST_CASE("Parser evaluates expressions with no variables") {
  ExpressionParser parser;

  auto expression = parser.Parser("2 + 3 * 4");
  std::unordered_map<std::string, double> values;

  CHECK(expression(values) == Approx(14.0));
}

TEST_CASE("Parser evaluates a single-variable expression") {
  ExpressionParser parser;
  auto expression = parser.Parser("health + 5");

  std::unordered_map<std::string, double> values{{"health", 10.0}};
  CHECK(expression(values) == Approx(15.0));

  values["health"] = 2.5;
  CHECK(expression(values) == Approx(7.5));
}

TEST_CASE("Parser handles precedence and parentheses") {
  ExpressionParser parser;
  auto expression = parser.Parser("(value * amount) + number / 2");

  std::unordered_map<std::string, double> values{
      {"value", 6.0}, {"amount", 3.0}, {"number", 8.0}};

  CHECK(expression(values) == Approx(22.0));
}

TEST_CASE("Parser supports repeated variables and ignores extra map entries") {
  ExpressionParser parser;
  auto expression = parser.Parser("x * x + x");

  std::unordered_map<std::string, double> values{{"x", 3.0}, {"unused", 999.0}};

  CHECK(expression(values) == Approx(12.0));
}

TEST_CASE("Parser supports identifier names with underscores and digits") {
  ExpressionParser parser;
  auto expression = parser.Parser("health_2 + stamina1 * 2");

  std::unordered_map<std::string, double> values{{"health_2", 4.5},
                                                 {"stamina1", 3.0}};

  CHECK(expression(values) == Approx(10.5));
}

TEST_CASE("Parser handles negative numbers", "[expression-parser]") {
  ExpressionParser parser;
  auto expression = parser.Parser("-5 + x");

  std::unordered_map<std::string, double> values{{"x", 2.0}};
  CHECK(expression(values) == Approx(-3.0));
}

TEST_CASE("Parser handles division by zero", "[expression-parser]") {
  ExpressionParser parser;
  auto expression = parser.Parser("x / y");

  std::unordered_map<std::string, double> values{{"x", 10.0}, {"y", 0.0}};
  const double result = expression(values);
  CHECK(std::isinf(result));
  CHECK(result > 0.0);
}

TEST_CASE("Parser asserts on empty expression",
          "[expression-parser][assert-empty]") {
  ExpressionParser parser;
  CHECK_THROWS(parser.Parser(""));
}

TEST_CASE("Parser asserts on invalid expression syntax",
          "[expression-parser][assert-syntax]") {
  ExpressionParser parser;
  CHECK_THROWS(parser.Parser("health +"));
}

TEST_CASE("Parser asserts when required variable is missing",
          "[expression-parser][assert-missing-var]") {
  ExpressionParser parser;
  auto expression = parser.Parser("health + stamina");
  std::unordered_map<std::string, double> values{{"health", 5.0}};
  CHECK_THROWS(expression(values));
}
