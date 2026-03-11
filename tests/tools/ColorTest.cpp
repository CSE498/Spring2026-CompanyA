// File: tests/tools/ColorTest.cpp
// Notes:
// - Do NOT define CATCH_CONFIG_MAIN here if another test file already defines it.

#include <array>
#include <optional>
#include <string>
#include <type_traits>

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/Color.hpp"

using cse498::Color;

TEST_CASE("Color is a lightweight value type", "[tools][color][traits]") {
    STATIC_REQUIRE(std::is_default_constructible_v<Color>);
    STATIC_REQUIRE(std::is_copy_constructible_v<Color>);
    STATIC_REQUIRE(std::is_copy_assignable_v<Color>);
    STATIC_REQUIRE(std::is_move_constructible_v<Color>);
    STATIC_REQUIRE(std::is_move_assignable_v<Color>);
}

TEST_CASE("Color default construction yields opaque black", "[tools][color][ctor]") {
    const Color color;

    REQUIRE(color.R() == 0);
    REQUIRE(color.G() == 0);
    REQUIRE(color.B() == 0);
    REQUIRE(color.A() == 255);
}

TEST_CASE("Color can be constructed from RGBA255 values", "[tools][color][rgba255]") {
    const Color color = Color::FromRGBA255(12, 34, 56, 78);

    REQUIRE(color.R() == 12);
    REQUIRE(color.G() == 34);
    REQUIRE(color.B() == 56);
    REQUIRE(color.A() == 78);

    const auto rgba = color.ToRGBA255();
    REQUIRE(rgba.r == 12);
    REQUIRE(rgba.g == 34);
    REQUIRE(rgba.b == 56);
    REQUIRE(rgba.a == 78);
}

TEST_CASE("Color can be constructed from RGB255 values with default alpha", "[tools][color][rgba255]") {
    const Color color = Color::FromRGB255(12, 34, 56);

    REQUIRE(color.R() == 12);
    REQUIRE(color.G() == 34);
    REQUIRE(color.B() == 56);
    REQUIRE(color.A() == 255);
}

TEST_CASE("Color can be constructed from normalized float values", "[tools][color][rgba01]") {
    const Color color = Color::FromRGBA01(1.0f, 0.5f, 0.0f, 0.25f);

    REQUIRE(color.R() == 255);
    REQUIRE(color.G() == 128);
    REQUIRE(color.B() == 0);
    REQUIRE(color.A() == 64);

    const auto rgba01 = color.ToRGBA01();
    REQUIRE(rgba01.r == Approx(1.0f));
    REQUIRE(rgba01.g == Approx(128.0f / 255.0f));
    REQUIRE(rgba01.b == Approx(0.0f));
    REQUIRE(rgba01.a == Approx(64.0f / 255.0f));
}

TEST_CASE("Color clamps normalized float inputs to [0, 1]", "[tools][color][rgba01]") {
    const Color color = Color::FromRGBA01(-1.0f, 2.0f, 0.25f, 5.0f);

    REQUIRE(color.R() == 0);
    REQUIRE(color.G() == 255);
    REQUIRE(color.B() == 64);
    REQUIRE(color.A() == 255);
}

TEST_CASE("Color setters update channels as expected", "[tools][color][setters]") {
    Color color;

    color.SetR(10);
    color.SetG(20);
    color.SetB(30);
    color.SetA(40);

    REQUIRE(color.R() == 10);
    REQUIRE(color.G() == 20);
    REQUIRE(color.B() == 30);
    REQUIRE(color.A() == 40);

    color.SetRGBA255(1, 2, 3, 4);
    REQUIRE(color.R() == 1);
    REQUIRE(color.G() == 2);
    REQUIRE(color.B() == 3);
    REQUIRE(color.A() == 4);

    color.SetRGBA01(1.0f, 0.0f, 0.5f, 1.0f);
    REQUIRE(color.R() == 255);
    REQUIRE(color.G() == 0);
    REQUIRE(color.B() == 128);
    REQUIRE(color.A() == 255);
}

TEST_CASE("Color ToArray returns RGBA in order", "[tools][color][array]") {
    const Color color = Color::FromRGBA255(9, 8, 7, 6);
    const std::array<std::uint8_t, 4> values = color.ToArray();

    REQUIRE(values == std::array<std::uint8_t, 4>{9, 8, 7, 6});
}

TEST_CASE("Color equality compares all channels", "[tools][color][equality]") {
    const Color a = Color::FromRGBA255(1, 2, 3, 4);
    const Color b = Color::FromRGBA255(1, 2, 3, 4);
    const Color c = Color::FromRGBA255(1, 2, 3, 5);

    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);
}

TEST_CASE("Color parses short and long hex forms", "[tools][color][hex]") {
    SECTION("#RGB") {
        const auto color = Color::FromHex("#f80");
        REQUIRE(color.has_value());
        REQUIRE(color->R() == 255);
        REQUIRE(color->G() == 136);
        REQUIRE(color->B() == 0);
        REQUIRE(color->A() == 255);
    }

    SECTION("#RGBA") {
        const auto color = Color::FromHex("#f804");
        REQUIRE(color.has_value());
        REQUIRE(color->R() == 255);
        REQUIRE(color->G() == 136);
        REQUIRE(color->B() == 0);
        REQUIRE(color->A() == 68);
    }

    SECTION("#RRGGBB") {
        const auto color = Color::FromHex("#ff8800");
        REQUIRE(color.has_value());
        REQUIRE(color->R() == 255);
        REQUIRE(color->G() == 136);
        REQUIRE(color->B() == 0);
        REQUIRE(color->A() == 255);
    }

    SECTION("#RRGGBBAA") {
        const auto color = Color::FromHex("#ff880040");
        REQUIRE(color.has_value());
        REQUIRE(color->R() == 255);
        REQUIRE(color->G() == 136);
        REQUIRE(color->B() == 0);
        REQUIRE(color->A() == 64);
    }

    SECTION("hex without leading #") {
        const auto color = Color::FromHex("336699");
        REQUIRE(color.has_value());
        REQUIRE(color->R() == 51);
        REQUIRE(color->G() == 102);
        REQUIRE(color->B() == 153);
        REQUIRE(color->A() == 255);
    }
}

TEST_CASE("Color rejects invalid hex strings", "[tools][color][hex]") {
    REQUIRE_FALSE(Color::FromHex("" ).has_value());
    REQUIRE_FALSE(Color::FromHex("#12").has_value());
    REQUIRE_FALSE(Color::FromHex("#12345").has_value());
    REQUIRE_FALSE(Color::FromHex("#zzzzzz").has_value());
}

TEST_CASE("Color formats hex strings", "[tools][color][hex]") {
    const Color color = Color::FromRGBA255(255, 136, 0, 64);

    REQUIRE(color.ToHex() == "#ff8800");
    REQUIRE(color.ToHex(true) == "#ff880040");
    REQUIRE(color.ToHex(false, true) == "#FF8800");
    REQUIRE(color.ToHex(true, true) == "#FF880040");
}

TEST_CASE("Color parses rgb() strings", "[tools][color][string]") {
    const auto color = Color::FromString("rgb(255, 128, 0)");

    REQUIRE(color.has_value());
    REQUIRE(color->R() == 255);
    REQUIRE(color->G() == 128);
    REQUIRE(color->B() == 0);
    REQUIRE(color->A() == 255);
}

TEST_CASE("Color parses rgba() strings", "[tools][color][string]") {
    SECTION("alpha as float") {
        const auto color = Color::FromString("rgba(255, 128, 0, 0.5)");
        REQUIRE(color.has_value());
        REQUIRE(color->R() == 255);
        REQUIRE(color->G() == 128);
        REQUIRE(color->B() == 0);
        REQUIRE(color->A() == 128);
    }

    SECTION("alpha as percent") {
        const auto color = Color::FromString("rgba(255, 128, 0, 25%)");
        REQUIRE(color.has_value());
        REQUIRE(color->R() == 255);
        REQUIRE(color->G() == 128);
        REQUIRE(color->B() == 0);
        REQUIRE(color->A() == 64);
    }
}

TEST_CASE("Color parses percentage rgb() channels", "[tools][color][string]") {
    const auto color = Color::FromString("rgb(100%, 50%, 0%)");

    REQUIRE(color.has_value());
    REQUIRE(color->R() == 255);
    REQUIRE(color->G() == 128);
    REQUIRE(color->B() == 0);
    REQUIRE(color->A() == 255);
}

TEST_CASE("Color parses named colors and transparent", "[tools][color][string]") {
    SECTION("named color") {
        const auto color = Color::FromString("orange");
        REQUIRE(color.has_value());
        REQUIRE(color->R() == 255);
        REQUIRE(color->G() == 165);
        REQUIRE(color->B() == 0);
        REQUIRE(color->A() == 255);
    }

    SECTION("transparent") {
        const auto color = Color::FromString("transparent");
        REQUIRE(color.has_value());
        REQUIRE(color->R() == 0);
        REQUIRE(color->G() == 0);
        REQUIRE(color->B() == 0);
        REQUIRE(color->A() == 0);
    }
}

TEST_CASE("Color FromString rejects invalid strings", "[tools][color][string]") {
    REQUIRE_FALSE(Color::FromString("").has_value());
    REQUIRE_FALSE(Color::FromString("rgb(255, 0)").has_value());
    REQUIRE_FALSE(Color::FromString("rgba(255, 0, 0)").has_value());
    REQUIRE_FALSE(Color::FromString("rgb(255, nope, 0)").has_value());
    REQUIRE_FALSE(Color::FromString("not-a-color").has_value());
}

TEST_CASE("Color string formatting matches stored values", "[tools][color][string]") {
    const Color color = Color::FromRGBA255(255, 128, 0, 64);

    REQUIRE(color.ToRgbString() == "rgb(255, 128, 0)");
    REQUIRE(color.ToRgbaString() == "rgba(255, 128, 0, 0.25098)");
}
