/**
 * @file TestWorldPath.cpp
 * @author Luke Antone
 *
 * Catch2 unit tests for WorldPath.
 */

#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/WorldPath.hpp"
#include "../../source/core/WorldPosition.hpp"

#include <sstream>
#include <vector>
#include <span>
#include <cmath>

namespace
{

[[nodiscard]] cse498::WorldPosition WP(double x, double y)
{
    return cse498::WorldPosition{x, y};
}

}

TEST_CASE("WorldPath: default construction", "[WorldPath]")
{
    cse498::WorldPath p;
    REQUIRE(p.Empty());
    REQUIRE(p.Size() == 0);
    REQUIRE(p.Length() == 0.0);
    REQUIRE_FALSE(p.SelfIntersects());
    REQUIRE_FALSE(p.FurthestPointPair().has_value());
}

TEST_CASE("WorldPath: construct from span", "[WorldPath]")
{
    const std::vector<cse498::WorldPosition> pts = {WP(0,0), WP(3,4), WP(3,0)};
    const cse498::WorldPath p{std::span<const cse498::WorldPosition>{pts}};

    REQUIRE_FALSE(p.Empty());
    REQUIRE(p.Size() == pts.size());
    REQUIRE(p.At(0).X() == Catch::Detail::Approx(0.0));
    REQUIRE(p.At(0).Y() == Catch::Detail::Approx(0.0));
}

TEST_CASE("WorldPath: AddPoint / Span / Clear", "[WorldPath]")
{
    cse498::WorldPath p;

    p.AddPoint(WP(1,2));
    p.AddPoint(WP(3,4));

    REQUIRE(p.Size() == 2);
    REQUIRE_FALSE(p.Empty());

    const auto span = p.Span();
    REQUIRE(span.size() == 2);

    REQUIRE(span[0].X() == Catch::Detail::Approx(1.0));
    REQUIRE(span[0].Y() == Catch::Detail::Approx(2.0));
    REQUIRE(span[1].X() == Catch::Detail::Approx(3.0));
    REQUIRE(span[1].Y() == Catch::Detail::Approx(4.0));

    p.Clear();

    REQUIRE(p.Empty());
    REQUIRE(p.Size() == 0);
}

TEST_CASE("WorldPath: At bounds checks", "[WorldPath]")
{
    cse498::WorldPath p;
    p.AddPoint(WP(0,0));

    REQUIRE_NOTHROW(p.At(0));
    REQUIRE_THROWS(p.At(1));
}

TEST_CASE("WorldPath: Length with 0/1/2+ points", "[WorldPath]")
{
    cse498::WorldPath p;

    REQUIRE(p.Length() == 0.0);

    p.AddPoint(WP(0,0));
    REQUIRE(p.Length() == 0.0);

    p.AddPoint(WP(3,4)); // distance = 5
    REQUIRE(p.Length() == Catch::Detail::Approx(5.0));

    p.AddPoint(WP(3,0)); // +4
    REQUIRE(p.Length() == Catch::Detail::Approx(9.0));
}

TEST_CASE("WorldPath: Length handles fractional positions", "[WorldPath]")
{
    cse498::WorldPath p;

    p.AddPoint(WP(0.5, 1.25));
    p.AddPoint(WP(3.5, 5.25)); // distance = 5

    REQUIRE(p.Length() == Catch::Detail::Approx(5.0));
}

TEST_CASE("WorldPath: SelfIntersects false for simple polyline", "[WorldPath]")
{
    cse498::WorldPath p;

    p.AddPoint(WP(0,0));
    p.AddPoint(WP(1,0));
    p.AddPoint(WP(2,0));
    p.AddPoint(WP(3,0));

    REQUIRE_FALSE(p.SelfIntersects());
}

TEST_CASE("WorldPath: SelfIntersects true for crossing segments", "[WorldPath]")
{
    cse498::WorldPath p;

    p.AddPoint(WP(0,0));
    p.AddPoint(WP(2,2));
    p.AddPoint(WP(0,2));
    p.AddPoint(WP(2,0));

    REQUIRE(p.SelfIntersects());
}

TEST_CASE("WorldPath: SelfIntersects true for immediate backtracking", "[WorldPath]")
{
    cse498::WorldPath p;

    p.AddPoint(WP(0,0));
    p.AddPoint(WP(1,0));
    p.AddPoint(WP(0,0));

    REQUIRE(p.SelfIntersects());
}

TEST_CASE("WorldPath: FurthestPointPair returns nullopt for <2 points", "[WorldPath]")
{
    cse498::WorldPath p;

    REQUIRE_FALSE(p.FurthestPointPair().has_value());

    p.AddPoint(WP(0,0));
    REQUIRE_FALSE(p.FurthestPointPair().has_value());
}

TEST_CASE("WorldPath: FurthestPointPair returns correct indices", "[WorldPath]")
{
    cse498::WorldPath p;

    p.AddPoint(WP(0,0));
    p.AddPoint(WP(0,3));
    p.AddPoint(WP(4,0));

    const auto pairOpt = p.FurthestPointPair();

    REQUIRE(pairOpt.has_value());

    const auto [i,j] = *pairOpt;

    REQUIRE(i < p.Size());
    REQUIRE(j < p.Size());
    REQUIRE(i != j);

    const double dx = p.At(i).X() - p.At(j).X();
    const double dy = p.At(i).Y() - p.At(j).Y();
    const double d = std::sqrt(dx*dx + dy*dy);

    REQUIRE(d == Catch::Detail::Approx(5.0));
}

TEST_CASE("WorldPath: Reverse reverses point order", "[WorldPath]")
{
    cse498::WorldPath p;

    p.AddPoint(WP(0,0));
    p.AddPoint(WP(1,0));
    p.AddPoint(WP(2,0));

    const double lenBefore = p.Length();

    p.Reverse();

    REQUIRE(p.Size() == 3);
    REQUIRE(p.At(0) == WP(2,0));
    REQUIRE(p.At(1) == WP(1,0));
    REQUIRE(p.At(2) == WP(0,0));

    REQUIRE(p.Length() == Catch::Detail::Approx(lenBefore));
}

TEST_CASE("WorldPath: Extend appends points and skips consecutive duplicates", "[WorldPath]")
{
    cse498::WorldPath a;

    a.AddPoint(WP(0,0));
    a.AddPoint(WP(1,0));

    cse498::WorldPath b;

    b.AddPoint(WP(1,0));
    b.AddPoint(WP(2,0));
    b.AddPoint(WP(3,0));

    a.Extend(b);

    REQUIRE(a.Size() == 4);

    REQUIRE(a.At(0) == WP(0,0));
    REQUIRE(a.At(1) == WP(1,0));
    REQUIRE(a.At(2) == WP(2,0));
    REQUIRE(a.At(3) == WP(3,0));
}

TEST_CASE("WorldPath: operator== compares paths by point sequence", "[WorldPath]")
{
    cse498::WorldPath a;

    a.AddPoint(WP(0,0));
    a.AddPoint(WP(1,1));

    cse498::WorldPath b;

    b.AddPoint(WP(0,0));
    b.AddPoint(WP(1,1));

    cse498::WorldPath c;

    c.AddPoint(WP(0,0));
    c.AddPoint(WP(1,0));

    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);

    b.Reverse();

    REQUIRE_FALSE(a == b);
}

TEST_CASE("WorldPath: operator<< prints basic info", "[WorldPath]")
{
    cse498::WorldPath p;

    p.AddPoint(WP(0,0));
    p.AddPoint(WP(3,4));

    std::ostringstream oss;

    oss << p;

    const std::string s = oss.str();

    REQUIRE(s.find("WorldPath") != std::string::npos);
    REQUIRE(s.find("points=") != std::string::npos);
    REQUIRE(s.find("length=") != std::string::npos);
}