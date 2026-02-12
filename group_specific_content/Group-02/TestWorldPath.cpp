/**
 * @file TestWorldPath.cpp
 * @author Luke Antone
 *
 * Catch2 unit tests for WorldPath.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "../WorldPath.h"                 // adjust path if your tests folder differs
#include "../../../source/core/WorldPosition.hpp"  // adjust if needed

#include <sstream>
#include <vector>

using namespace cse498;

namespace
{
    // TODO: Adjust this ONE helper to match your WorldPosition API.
    // Common possibilities:
    //   return WorldPosition{x, y};
    //   return WorldPosition(x, y);
    //   return WorldPosition::FromXY(x, y);
    [[nodiscard]] WorldPosition WP(double x, double y)
    {
        return WorldPosition{x, y};
    }
}

TEST_CASE("WorldPath: default construction", "[WorldPath]")
{
    WorldPath p;
    REQUIRE(p.Empty());
    REQUIRE(p.Size() == 0);
    REQUIRE(p.Length() == 0.0);
    REQUIRE_FALSE(p.SelfIntersects());
    REQUIRE_FALSE(p.FurthestPointPair().has_value());
}

TEST_CASE("WorldPath: construct from span", "[WorldPath]")
{
    const std::vector<WorldPosition> pts = {WP(0, 0), WP(3, 4), WP(3, 0)};
    const WorldPath p{std::span<const WorldPosition>{pts}};
    REQUIRE_FALSE(p.Empty());
    REQUIRE(p.Size() == pts.size());
    REQUIRE(p.At(0).X() == Catch::Approx(0.0));
    REQUIRE(p.At(0).Y() == Catch::Approx(0.0));
}

TEST_CASE("WorldPath: AddPoint / Points / Clear", "[WorldPath]")
{
    WorldPath p;
    p.AddPoint(WP(1, 2));
    p.AddPoint(WP(3, 4));

    REQUIRE(p.Size() == 2);
    REQUIRE_FALSE(p.Empty());

    const auto& v = p.Points();
    REQUIRE(v.size() == 2);
    REQUIRE(v[0].X() == Catch::Approx(1.0));
    REQUIRE(v[0].Y() == Catch::Approx(2.0));
    REQUIRE(v[1].X() == Catch::Approx(3.0));
    REQUIRE(v[1].Y() == Catch::Approx(4.0));

    p.Clear();
    REQUIRE(p.Empty());
    REQUIRE(p.Size() == 0);
}

TEST_CASE("WorldPath: At bounds checks", "[WorldPath]")
{
    WorldPath p;
    p.AddPoint(WP(0, 0));

    REQUIRE_NOTHROW(p.At(0));
    REQUIRE_THROWS(p.At(1)); // std::vector::at throws on OOB
}

TEST_CASE("WorldPath: Length with 0/1/2+ points", "[WorldPath]")
{
    WorldPath p;
    REQUIRE(p.Length() == 0.0);

    p.AddPoint(WP(0, 0));
    REQUIRE(p.Length() == 0.0);

    p.AddPoint(WP(3, 4)); // distance = 5
    REQUIRE(p.Length() == Catch::Approx(5.0));

    p.AddPoint(WP(3, 0)); // +4
    REQUIRE(p.Length() == Catch::Approx(9.0));
}

TEST_CASE("WorldPath: SelfIntersects false for simple polyline", "[WorldPath]")
{
    WorldPath p;
    p.AddPoint(WP(0, 0));
    p.AddPoint(WP(1, 0));
    p.AddPoint(WP(2, 0));
    p.AddPoint(WP(3, 0));

    REQUIRE_FALSE(p.SelfIntersects());
}

TEST_CASE("WorldPath: SelfIntersects true for crossing segments", "[WorldPath]")
{
    // This creates two segments that cross:
    // (0,0)->(2,2) and (0,2)->(2,0)
    WorldPath p;
    p.AddPoint(WP(0, 0));
    p.AddPoint(WP(2, 2));
    p.AddPoint(WP(0, 2));
    p.AddPoint(WP(2, 0));

    REQUIRE(p.SelfIntersects());
}

TEST_CASE("WorldPath: FurthestPointPair returns nullopt for <2 points", "[WorldPath]")
{
    WorldPath p;
    REQUIRE_FALSE(p.FurthestPointPair().has_value());

    p.AddPoint(WP(0, 0));
    REQUIRE_FALSE(p.FurthestPointPair().has_value());
}

TEST_CASE("WorldPath: FurthestPointPair returns correct indices", "[WorldPath]")
{
    // Points: 0:(0,0), 1:(0,3), 2:(4,0)  -> farthest pair is (1,2) dist=5 OR (0,2) dist=4, (0,1) dist=3
    WorldPath p;
    p.AddPoint(WP(0, 0));
    p.AddPoint(WP(0, 3));
    p.AddPoint(WP(4, 0));

    const auto pairOpt = p.FurthestPointPair();
    REQUIRE(pairOpt.has_value());

    const auto [i, j] = *pairOpt;
    REQUIRE(i < p.Size());
    REQUIRE(j < p.Size());
    REQUIRE(i != j);

    // Check that the chosen pair is actually the farthest by verifying distance is 5.
    const double dx = p.At(i).X() - p.At(j).X();
    const double dy = p.At(i).Y() - p.At(j).Y();
    const double d = std::sqrt(dx * dx + dy * dy);
    REQUIRE(d == Catch::Approx(5.0));
}

TEST_CASE("WorldPath: operator<< prints basic info", "[WorldPath]")
{
    WorldPath p;
    p.AddPoint(WP(0, 0));
    p.AddPoint(WP(3, 4)); // length 5

    std::ostringstream oss;
    oss << p;

    const std::string s = oss.str();
    REQUIRE(s.find("WorldPath") != std::string::npos);
    REQUIRE(s.find("points=") != std::string::npos);
    REQUIRE(s.find("length=") != std::string::npos);
}
