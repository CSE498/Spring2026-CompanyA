#include "../../source/tools/InteractiveWorld/ExpressionParser.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

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
