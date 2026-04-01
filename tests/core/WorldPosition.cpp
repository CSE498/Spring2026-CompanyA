#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/core/WorldPosition.hpp"

#include <functional>


using cse498::WorldPosition;

TEST_CASE("Test WorldPosition Constructors", "[core]")
{
  WorldPosition wp1;       // Default Constructor
  WorldPosition wp2(4,5);  // Direct Constructor
  WorldPosition wp3(0,0);
  WorldPosition wp4(wp2);  // Copy Constructor

  // Test basic comparisons.
  CHECK(wp1 == wp1);
  CHECK(wp1 != wp2);
  CHECK(wp1 == wp3);
  CHECK(wp1 != wp4);
  CHECK(wp2 == wp4);

  wp4 = WorldPosition{3,4};
  CHECK(wp2 != wp4);
  wp2 = wp4;
  CHECK(wp2 == wp4);

  // Test CellX() and CellY()
  CHECK(wp2.CellX() == 3);
  CHECK(wp2.CellY() == 4);

  wp4 = WorldPosition(1000, 2000);
  CHECK(wp4.CellX() == 1000);
  CHECK(wp4.CellY() == 2000);

  // Test more direct setting of values.
  wp1.Set(7,9.5);
  CHECK(wp1.X() == 7.0);
  CHECK(wp1.Y() == 9.5);
  CHECK(wp1.CellX() == 7);
  CHECK(wp1.CellY() == 9);

  CHECK(wp1.Set(10,10).Up().Y() == 9);
}

TEST_CASE("WorldPosition Round truncates to the containing cell", "[core, WorldPosition]")
{
  WorldPosition pos(3.99, 4.01);

  WorldPosition& rounded = pos.Round();

  CHECK(&rounded == &pos);
  CHECK(pos.X() == 3.0);
  CHECK(pos.Y() == 4.0);
  CHECK(pos.CellX() == 3);
  CHECK(pos.CellY() == 4);
}

TEST_CASE("Free Round returns a rounded copy without mutating the source", "[core, WorldPosition]")
{
  const WorldPosition original(8.75, 2.25);

  const WorldPosition rounded = cse498::Round(original);

  CHECK(original.X() == Approx(8.75));
  CHECK(original.Y() == Approx(2.25));
  CHECK(rounded.X() == 8.0);
  CHECK(rounded.Y() == 2.0);
}

TEST_CASE("Quantize keeps positions equal within bins", "[core, WorldPosition]")
{

  const double epsilon = 1e-9;
  const WorldPosition base(1.0, 2.0);
  const WorldPosition near_match(1.0 + 0.49 * epsilon, 2.0 - 0.49 * epsilon);

  CHECK(WorldPosition::Quantize(base.X()) == WorldPosition::Quantize(near_match.X()));
  CHECK(WorldPosition::Quantize(base.Y()) == WorldPosition::Quantize(near_match.Y()));
  CHECK(base == near_match);
}

TEST_CASE("Quantize separates positions that cross an epsilon bucket boundary", "[core, WorldPosition]")
{

  const double epsilon = 1e-9;
  const WorldPosition base(1.0, 2.0);
  const WorldPosition shifted(1.0 + 1.1 * epsilon, 2.0);

  CHECK(WorldPosition::Quantize(base.X()) != WorldPosition::Quantize(shifted.X()));
  CHECK(base != shifted);
}

TEST_CASE("WorldPosition hash follows quantized equality", "[core, WorldPosition]")
{

  const double epsilon = 1e-9;
  const WorldPosition a(5.0, 9.0);
  const WorldPosition b(5.0 + 0.49 * epsilon, 9.0 - 0.49 * epsilon);

  REQUIRE(a == b);
  CHECK(std::hash<WorldPosition>{}(a) == std::hash<WorldPosition>{}(b));
}

TEST_CASE("StepPolar moves along positive X at zero radians", "[core, WorldPosition]")
{
  const WorldPosition start(2.0, 3.0);

  const WorldPosition end = start.StepPolar(4.5, 0.0);

  CHECK(end.X() == Approx(6.5));
  CHECK(end.Y() == Approx(3.0));
}

TEST_CASE("StepPolar moves along positive Y at pi over two radians", "[core, WorldPosition]")
{
  constexpr double half_pi = 1.5707963267948966;
  const WorldPosition start(1.25, -2.5);

  const WorldPosition end = start.StepPolar(3.0, half_pi);

  CHECK(end.X() == Approx(1.25).margin(1e-12));
  CHECK(end.Y() == Approx(0.5));
}

TEST_CASE("StepPolar with zero length leaves the position unchanged", "[core, WorldPosition]")
{
  const WorldPosition start(-7.0, 4.0);

  const WorldPosition end = start.StepPolar(0.0, 1.2345);

  CHECK(end.X() == Approx(start.X()));
  CHECK(end.Y() == Approx(start.Y()));
}
