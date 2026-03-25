/**
 * @file TestPathVector.cpp
 * @author Logan Rimarcik
 */



#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "tools/PathVector.hpp"
#include "core/WorldPosition.hpp"
#include <iostream>
#include <numbers>
using cse498::PathVector;
using std::numbers::pi;
using cse498::WorldPosition;



TEST_CASE("PathVector Constructors", "[constructor]")
{
    {
        PathVector v(3.0, 4.0);

        CHECK(v.X() == 3.0);
        CHECK(v.Y() == 4.0);
    }
    {
        PathVector v(0.0, 0.0);

        CHECK(v.X() == 0.0);
        CHECK(v.Y() == 0.0);
    }
    {
        WorldPosition from(1.0, 2.0);
        WorldPosition to(4.0, 6.0);
        PathVector v(from, to);

        CHECK(v.X() == 3.0);
        CHECK(v.Y() == 4.0);
    }
    {
        WorldPosition from(5.0, 5.0);
        WorldPosition to(5.0, 5.0);
        PathVector v(from, to);

        CHECK(v.X() == 0.0);
        CHECK(v.Y() == 0.0);
    }

}

TEST_CASE("Operations + -", "[operations]")
{
    // not complicated hard to mess up if it fails these
    PathVector v1(1.0, 2.0);
    PathVector v2(3.0, 4.0);
    PathVector v3 = v1 + v2;
    PathVector v4 = v1 - v2;
    PathVector v5 = v1 - v1;
    CHECK(v3.X() == 4.0);
    CHECK(v3.Y() == 6.0);
    CHECK(v4.X() == -2.0);
    CHECK(v4.Y() == -2.0);
    CHECK(v5.X() == 0.0);
    CHECK(v5.Y() == 0.0);
}

TEST_CASE("Operations mult, dot, magnitude", "[operations]")
{
    {
        PathVector v1(1.0, 2.0);
        PathVector v2(3.0, 4.0);

        CHECK(v1.Dot(v2) == Approx(11.0));
        CHECK(v1.GetMagnitude() == Approx(std::sqrt(5)));
        CHECK(v2.GetMagnitude() == Approx(5.0));

        PathVector result = v1.Mult(v2);
        CHECK(result.X() == Approx(3.0));
        CHECK(result.Y() == Approx(8.0));
    }

    {
        PathVector v1(-2.0, 3.0);
        PathVector v2(4.0, -5.0);

        CHECK(v1.Dot(v2) == Approx(-23.0));
        CHECK(v1.GetMagnitude() == Approx(std::sqrt(13)));
        CHECK(v2.GetMagnitude() == Approx(std::sqrt(41)));

        PathVector result = v1.Mult(v2);
        CHECK(result.X() == Approx(-8.0));
        CHECK(result.Y() == Approx(-15.0));
    }

    {
        PathVector v1(0.0, 0.0);
        PathVector v2(5.0, 7.0);

        CHECK(v1.Dot(v2) == Approx(0.0));
        CHECK(v1.GetMagnitude() == Approx(0.0));
        CHECK(v2.GetMagnitude() == Approx(std::sqrt(74)));

        PathVector result = v1.Mult(v2);
        CHECK(result.X() == Approx(0.0));
        CHECK(result.Y() == Approx(0.0));
    }

}


TEST_CASE("Normalize", "[function]")
{
    PathVector v(3.0, 4.0);
    v.Normalize();
    CHECK(v.X() == 0.6);
    CHECK(v.Y() == 0.8);
}

TEST_CASE("Scale", "[function]")
{
    PathVector v(3.0, 4.0);
    v.Scale(2.0);
    CHECK(v.X() == 6.0);
    CHECK(v.Y() == 8.0);
}

TEST_CASE("Rotate", "[function]")
{
    {
        PathVector v(0,1);
        v.Rotate(std::numbers::pi / 2.0);
        CHECK(v.X() == Approx(-1));
        CHECK(v.Y() == Approx(0).margin(1e-9));
    }
    {
        PathVector v(1, 0);
        v.Rotate(std::numbers::pi);

        CHECK(v.X() == Approx(-1.0));
        CHECK(v.Y() == Approx(0.0).margin(1e-9));
    }
    {
        PathVector v(3, 4); // magnitude = 5
        double originalMag = v.GetMagnitude();

        v.Rotate(std::numbers::pi / 3.0);

        CHECK(v.GetMagnitude() == Approx(originalMag));
        CHECK(v.X() == Approx(-1.9641));
        CHECK(v.Y() == Approx(4.59808));
    }



}

TEST_CASE("Angle", "[function]")
{
    {
        PathVector v(1, 0);
        CHECK(v.GetAngle() == Approx(0.0));
        v.Rotate(pi / 4.0);
        CHECK(v.GetAngle() == Approx(std::numbers::pi / 4.0));
    }
    {
        PathVector v(44, 0);

        v.Rotate(pi / 2.0);
        CHECK(v.GetAngle() == Approx(std::numbers::pi / 2.0).margin(1e-9));
    }
    {
        PathVector v(1, std::sqrt(3));
        CHECK(v.GetAngle() == Approx(std::numbers::pi / 3.0));
    }
    {
        PathVector v(std::sqrt(3), 1);
        CHECK(v.GetAngle() == Approx(std::numbers::pi / 6.0));
    }
}

TEST_CASE("Projection", "[function]")
{
    {
        PathVector v(3.0, -4.0);
        WorldPosition wp(33.0, 0.0);

        PathVector proj1 = v.Project(wp);
        PathVector proj2 = v.Project(PathVector(wp.X(), wp.Y()));

        CHECK(proj1.X() == Approx(proj2.X()));
        CHECK(proj1.Y() == Approx(proj2.Y()));
        CHECK(proj1.X() == Approx(11.88));
        CHECK(proj1.Y() == Approx(-15.84));
    }
    {
        PathVector a(-77.0, 4.0);
        WorldPosition b(1.0, 4.0);

        PathVector proj = a.Project(b);

        CHECK(proj.X() == Approx(0.790075693));
        CHECK(proj.Y() == Approx(-0.04104289318));
        // expected = (a·b / b·b) * b

    }
    {
        PathVector a(-1, -3.533);
        WorldPosition b(1.4, 1.0); // world position tests path vector

        PathVector proj = a.Project(b);
        CHECK(proj.X() == Approx(0.365893));
        CHECK(proj.Y() == Approx(1.2927));
    }

}


TEST_CASE("Other Operations", "[operations]")
{
    {
        WorldPosition p(3.0, 4.0);
        PathVector v(1.5, -2.0);

        WorldPosition result = p + v;

        REQUIRE(result.X() == Approx(4.5));
        REQUIRE(result.Y() == Approx(2.0));
    }
    {
        WorldPosition p(3.0, 4.0);
        PathVector v(1.5, -2.0);

        WorldPosition result = p - v;

        REQUIRE(result.X() == Approx(1.5));
        REQUIRE(result.Y() == Approx(6.0));
    }
    {
        WorldPosition p1(5.0, 7.0);
        WorldPosition p2(2.0, 3.0);

        PathVector result = p1 - p2;

        REQUIRE(result.X() == Approx(3.0));
        REQUIRE(result.Y() == Approx(4.0));

    }
    {
        WorldPosition p(10.0, 10.0);
        PathVector v(2.0, 3.0);

        WorldPosition moved = p + v;
        PathVector back = moved - p;

        REQUIRE(back.X() == Approx(v.X()));
        REQUIRE(back.Y() == Approx(v.Y()));
    }
}