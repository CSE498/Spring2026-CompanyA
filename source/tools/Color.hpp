/**
 * @file Color.hpp
 * @brief Defines a lightweight RGBA color value type and parsing/formatting utilities.
 *
 * This header provides the @ref cse498::Color class, a small self-contained utility
 * for representing colors in 8-bit RGBA form.
 *
 * The class supports:
 * - direct construction from 8-bit channel values,
 * - construction from normalized floating-point channel values in the range [0, 1],
 * - conversion to and from hexadecimal color strings,
 * - parsing of CSS-like @c rgb(...) and @c rgba(...) strings,
 * - parsing of a small built-in set of named colors,
 * - formatting back to common string representations.
 *
 * The implementation is intentionally independent of any graphics or UI framework.
 * Platform-specific adapters should be implemented outside this file.
 */

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>

namespace cse498 {

/**
 * @brief Represents a color using 8-bit RGBA channels.
 *
 * @details
 * The internal storage format is always four 8-bit channels:
 * - red
 * - green
 * - blue
 * - alpha
 *
 * The default value is opaque black:
 * - red   = 0
 * - green = 0
 * - blue  = 0
 * - alpha = 255
 *
 * This type is designed to behave like a lightweight value object and is suitable
 * for passing by value in most cases.
 *
 * Supported creation methods include:
 * - direct 8-bit RGBA construction,
 * - normalized floating-point construction,
 * - hexadecimal parsing,
 * - CSS-style string parsing,
 * - named-color lookup.
 *
 * @note
 * This class does not depend on external rendering libraries.
 * Conversion to framework-specific color types should be implemented separately.
 */
class Color {
public:
    /**
     * @brief Stores RGBA channels as 8-bit integer values.
     *
     * Each component is expected to be in the range [0, 255].
     */
    struct RGBA255 {
        std::uint8_t r; /**< Red channel in [0, 255]. */
        std::uint8_t g; /**< Green channel in [0, 255]. */
        std::uint8_t b; /**< Blue channel in [0, 255]. */
        std::uint8_t a; /**< Alpha channel in [0, 255]. */
    };

    /**
     * @brief Stores RGBA channels as normalized floating-point values.
     *
     * Each component is conceptually in the range [0.0, 1.0].
     * Values produced by @ref ToRGBA01 always fall within that range.
     */
    struct RGBA01 {
        float r; /**< Red channel in [0.0, 1.0]. */
        float g; /**< Green channel in [0.0, 1.0]. */
        float b; /**< Blue channel in [0.0, 1.0]. */
        float a; /**< Alpha channel in [0.0, 1.0]. */
    };

    /**
     * @brief Constructs an opaque black color.
     *
     * Equivalent to:
     * @code
     * Color(0, 0, 0, 255)
     * @endcode
     */
    constexpr Color() = default;

    /**
     * @brief Constructs a color from 8-bit RGBA channel values.
     *
     * @param r Red channel in [0, 255].
     * @param g Green channel in [0, 255].
     * @param b Blue channel in [0, 255].
     * @param a Alpha channel in [0, 255]. Defaults to 255 (fully opaque).
     */
    constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) noexcept :
        r_(r), g_(g), b_(b), a_(a) {}

    /**
     * @brief Creates a color from 8-bit RGBA channel values.
     *
     * @param r Red channel in [0, 255].
     * @param g Green channel in [0, 255].
     * @param b Blue channel in [0, 255].
     * @param a Alpha channel in [0, 255]. Defaults to 255.
     * @return A color with the specified channels.
     */
    [[nodiscard]] static constexpr Color FromRGBA255(std::uint8_t r, std::uint8_t g, std::uint8_t b,
                                                     std::uint8_t a = 255) noexcept {
        return Color(r, g, b, a);
    }

    /**
     * @brief Creates an opaque color from 8-bit RGB channel values.
     *
     * @param r Red channel in [0, 255].
     * @param g Green channel in [0, 255].
     * @param b Blue channel in [0, 255].
     * @return A color with alpha set to 255.
     */
    [[nodiscard]] static constexpr Color FromRGB255(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
        return Color(r, g, b, 255);
    }

    /**
     * @brief Creates a color from integral channel values with byte clamping.
     *
     * @tparam T Any integral type.
     * @param r Red channel value.
     * @param g Green channel value.
     * @param b Blue channel value.
     * @param a Alpha channel value. Defaults to 255 converted to @p T.
     * @return A color whose channels are clamped into the range [0, 255].
     *
     * @details
     * Values below 0 are clamped to 0. Values above 255 are clamped to 255.
     * This function is useful when color input originates from wider or signed
     * integer types.
     */
    template<std::integral T>
    [[nodiscard]] static constexpr Color FromRGBIntegral(T r, T g, T b, T a = static_cast<T>(255)) noexcept {
        return Color(ClampToByte(r), ClampToByte(g), ClampToByte(b), ClampToByte(a));
    }

    /**
     * @brief Creates a color from normalized floating-point channel values.
     *
     * @tparam T Any floating-point type.
     * @param r Red channel, expected in [0, 1].
     * @param g Green channel, expected in [0, 1].
     * @param b Blue channel, expected in [0, 1].
     * @param a Alpha channel, expected in [0, 1]. Defaults to 1.
     * @return A color converted to 8-bit RGBA storage.
     *
     * @details
     * Values are converted through @ref FromRGBA01. Inputs are clamped into
     * [0, 1] before conversion to byte channels.
     */
    template<std::floating_point T>
    [[nodiscard]] static Color FromRGBNormalized(T r, T g, T b, T a = static_cast<T>(1)) noexcept {
        return FromRGBA01(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), static_cast<float>(a));
    }

    /**
     * @brief Creates a color from normalized RGBA floating-point values.
     *
     * @param r Red channel, expected in [0.0, 1.0].
     * @param g Green channel, expected in [0.0, 1.0].
     * @param b Blue channel, expected in [0.0, 1.0].
     * @param a Alpha channel, expected in [0.0, 1.0]. Defaults to 1.0.
     * @return A color converted to 8-bit RGBA storage.
     *
     * @details
     * Each component is clamped to [0.0, 1.0] and then rounded to the nearest
     * 8-bit channel value.
     */
    [[nodiscard]] static Color FromRGBA01(float r, float g, float b, float a = 1.0f) noexcept {
        return Color(Float01ToByte(r), Float01ToByte(g), Float01ToByte(b), Float01ToByte(a));
    }

    /**
     * @brief Creates an opaque color from normalized RGB floating-point values.
     *
     * @param r Red channel, expected in [0.0, 1.0].
     * @param g Green channel, expected in [0.0, 1.0].
     * @param b Blue channel, expected in [0.0, 1.0].
     * @return A color with alpha set to 1.0 / 255.
     */
    [[nodiscard]] static Color FromRGB01(float r, float g, float b) noexcept { return FromRGBA01(r, g, b, 1.0f); }

    /**
     * @brief Parses a hexadecimal color string.
     *
     * @param text Input text. Leading and trailing ASCII whitespace is ignored.
     * @return A parsed color on success, or @c std::nullopt on failure.
     *
     * @details
     * Supported forms are:
     * - @c #RGB
     * - @c #RGBA
     * - @c #RRGGBB
     * - @c #RRGGBBAA
     * - the same forms without the leading @c '#'
     *
     * Short forms expand each nibble by duplication, for example:
     * - @c #f80  -> @c #ff8800
     * - @c #f804 -> @c #ff880044
     *
     * @note
     * Hexadecimal digits are accepted in either lowercase or uppercase.
     */
    [[nodiscard]] static constexpr std::optional<Color> FromHex(std::string_view text) noexcept {
        const std::string_view s = Trim(text);
        if (s.empty()) {
            return std::nullopt;
        }

        std::size_t pos = 0;
        if (s[0] == '#') {
            pos = 1;
        }

        const std::size_t len = s.size() - pos;
        if (len == 3 || len == 4) {
            const auto r = HexDigit(s[pos + 0]);
            const auto g = HexDigit(s[pos + 1]);
            const auto b = HexDigit(s[pos + 2]);
            const auto a = (len == 4) ? HexDigit(s[pos + 3]) : std::optional<std::uint8_t>(15);

            if (!r || !g || !b || !a) {
                return std::nullopt;
            }

            return Color(ExpandNibble(*r), ExpandNibble(*g), ExpandNibble(*b), ExpandNibble(*a));
        }

        if (len == 6 || len == 8) {
            const auto r = HexByte(s[pos + 0], s[pos + 1]);
            const auto g = HexByte(s[pos + 2], s[pos + 3]);
            const auto b = HexByte(s[pos + 4], s[pos + 5]);
            const auto a = (len == 8) ? HexByte(s[pos + 6], s[pos + 7]) : std::optional<std::uint8_t>(255);

            if (!r || !g || !b || !a) {
                return std::nullopt;
            }

            return Color(*r, *g, *b, *a);
        }

        return std::nullopt;
    }

    /**
     * @brief Parses a color from a general string representation.
     *
     * @param text Input text. Leading and trailing ASCII whitespace is ignored.
     * @return A parsed color on success, or @c std::nullopt on failure.
     *
     * @details
     * Supported inputs include:
     * - hexadecimal color strings accepted by @ref FromHex,
     * - @c rgb(...) function syntax,
     * - @c rgba(...) function syntax,
     * - the special keyword @c transparent,
     * - a small built-in set of named colors.
     *
     * Parsing is case-insensitive for:
     * - function prefixes such as @c rgb and @c rgba,
     * - named colors,
     * - the keyword @c transparent.
     *
     * Examples:
     * @code
     * Color::FromString("#ff8800");
     * Color::FromString("rgb(255, 128, 0)");
     * Color::FromString("rgba(255, 128, 0, 0.5)");
     * Color::FromString("rgba(255, 128, 0, 25%)");
     * Color::FromString("orange");
     * Color::FromString("transparent");
     * @endcode
     */
    [[nodiscard]] static std::optional<Color> FromString(std::string_view text) noexcept {
        const std::string_view s = Trim(text);
        if (s.empty()) {
            return std::nullopt;
        }

        if (StartsWithInsensitive(s, "#")) {
            return FromHex(s);
        }

        if (StartsWithInsensitive(s, "rgb(")) {
            return ParseRgbFunc(s, false);
        }

        if (StartsWithInsensitive(s, "rgba(")) {
            return ParseRgbFunc(s, true);
        }

        if (EqualsInsensitive(s, "transparent")) {
            return Color(0, 0, 0, 0);
        }

        if (const auto named = ParseNamedColor(s)) {
            return named;
        }

        return std::nullopt;
    }

    /**
     * @brief Returns the red channel.
     * @return The red channel in [0, 255].
     */
    [[nodiscard]] constexpr std::uint8_t R() const noexcept { return r_; }

    /**
     * @brief Returns the green channel.
     * @return The green channel in [0, 255].
     */
    [[nodiscard]] constexpr std::uint8_t G() const noexcept { return g_; }

    /**
     * @brief Returns the blue channel.
     * @return The blue channel in [0, 255].
     */
    [[nodiscard]] constexpr std::uint8_t B() const noexcept { return b_; }

    /**
     * @brief Returns the alpha channel.
     * @return The alpha channel in [0, 255].
     */
    [[nodiscard]] constexpr std::uint8_t A() const noexcept { return a_; }

    /**
     * @brief Sets the red channel.
     * @param value New red channel value in [0, 255].
     */
    constexpr void SetR(std::uint8_t value) noexcept { r_ = value; }

    /**
     * @brief Sets the green channel.
     * @param value New green channel value in [0, 255].
     */
    constexpr void SetG(std::uint8_t value) noexcept { g_ = value; }

    /**
     * @brief Sets the blue channel.
     * @param value New blue channel value in [0, 255].
     */
    constexpr void SetB(std::uint8_t value) noexcept { b_ = value; }

    /**
     * @brief Sets the alpha channel.
     * @param value New alpha channel value in [0, 255].
     */
    constexpr void SetA(std::uint8_t value) noexcept { a_ = value; }

    /**
     * @brief Sets all channels from 8-bit RGBA values.
     *
     * @param r Red channel in [0, 255].
     * @param g Green channel in [0, 255].
     * @param b Blue channel in [0, 255].
     * @param a Alpha channel in [0, 255]. Defaults to 255.
     */
    constexpr void SetRGBA255(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) noexcept {
        r_ = r;
        g_ = g;
        b_ = b;
        a_ = a;
    }

    /**
     * @brief Sets all channels from normalized floating-point values.
     *
     * @param r Red channel, expected in [0.0, 1.0].
     * @param g Green channel, expected in [0.0, 1.0].
     * @param b Blue channel, expected in [0.0, 1.0].
     * @param a Alpha channel, expected in [0.0, 1.0]. Defaults to 1.0.
     *
     * @details
     * Each input is clamped into [0.0, 1.0] and converted to an 8-bit channel
     * using rounding.
     */
    void SetRGBA01(float r, float g, float b, float a = 1.0f) noexcept {
        r_ = Float01ToByte(r);
        g_ = Float01ToByte(g);
        b_ = Float01ToByte(b);
        a_ = Float01ToByte(a);
    }

    /**
     * @brief Returns the color as an 8-bit RGBA aggregate.
     * @return A @ref RGBA255 struct containing the current channels.
     */
    [[nodiscard]] constexpr RGBA255 ToRGBA255() const noexcept { return RGBA255{r_, g_, b_, a_}; }

    /**
     * @brief Returns the color channels as a fixed array in RGBA order.
     * @return An array containing { red, green, blue, alpha }.
     */
    [[nodiscard]] constexpr std::array<std::uint8_t, 4> ToArray() const noexcept { return {r_, g_, b_, a_}; }

    /**
     * @brief Returns the color as normalized floating-point RGBA values.
     * @return A @ref RGBA01 struct with all channels in [0.0, 1.0].
     */
    [[nodiscard]] RGBA01 ToRGBA01() const noexcept {
        return RGBA01{ByteToFloat01(r_), ByteToFloat01(g_), ByteToFloat01(b_), ByteToFloat01(a_)};
    }

    /**
     * @brief Formats the color as a hexadecimal string.
     *
     * @param include_alpha If @c true, append the alpha channel as @c AA.
     * @param uppercase If @c true, use uppercase hexadecimal digits.
     * @return A string in the form @c #RRGGBB or @c #RRGGBBAA.
     *
     * @note
     * The leading @c '#' is always included in the output.
     */
    [[nodiscard]] std::string ToHex(bool include_alpha = false, bool uppercase = false) const {
        std::string out;
        out.reserve(include_alpha ? 9 : 7);
        out.push_back('#');
        AppendHexByte(out, r_, uppercase);
        AppendHexByte(out, g_, uppercase);
        AppendHexByte(out, b_, uppercase);
        if (include_alpha) {
            AppendHexByte(out, a_, uppercase);
        }
        return out;
    }

    /**
     * @brief Formats the color as an @c rgb(...) string.
     * @return A string in the form @c rgb(r, g, b).
     *
     * @details
     * The alpha channel is not included in this representation.
     */
    [[nodiscard]] std::string ToRgbString() const {
        return "rgb(" + std::to_string(static_cast<unsigned>(r_)) + ", " + std::to_string(static_cast<unsigned>(g_)) +
               ", " + std::to_string(static_cast<unsigned>(b_)) + ")";
    }

    /**
     * @brief Formats the color as an @c rgba(...) string.
     * @return A string in the form @c rgba(r, g, b, a).
     *
     * @details
     * The alpha channel is emitted as a normalized floating-point string.
     */
    [[nodiscard]] std::string ToRgbaString() const {
        return "rgba(" + std::to_string(static_cast<unsigned>(r_)) + ", " + std::to_string(static_cast<unsigned>(g_)) +
               ", " + std::to_string(static_cast<unsigned>(b_)) + ", " + AlphaString() + ")";
    }

    /**
     * @brief Compares two colors for exact channel equality.
     *
     * @param other Another color.
     * @return @c true if all four channels are equal; otherwise @c false.
     */
    [[nodiscard]] constexpr bool operator==(const Color& other) const noexcept = default;

private:
    /**
     * @brief Entry used by the built-in named-color lookup table.
     */
    struct NamedColorEntry {
        std::string_view name; /**< Case-insensitive color name. */
        std::uint8_t r;        /**< Red channel. */
        std::uint8_t g;        /**< Green channel. */
        std::uint8_t b;        /**< Blue channel. */
        std::uint8_t a;        /**< Alpha channel. */
    };

    std::uint8_t r_ = 0;   /**< Stored red channel. */
    std::uint8_t g_ = 0;   /**< Stored green channel. */
    std::uint8_t b_ = 0;   /**< Stored blue channel. */
    std::uint8_t a_ = 255; /**< Stored alpha channel. */

    /**
     * @brief Small built-in set of named colors.
     *
     * @details
     * The lookup is case-insensitive and intended as a practical convenience,
     * not as a complete CSS color keyword database.
     */
    static constexpr std::array<NamedColorEntry, 20> kNamedColors{{
            {"black", 0, 0, 0, 255},      {"white", 255, 255, 255, 255}, {"red", 255, 0, 0, 255},
            {"green", 0, 128, 0, 255},    {"blue", 0, 0, 255, 255},      {"yellow", 255, 255, 0, 255},
            {"cyan", 0, 255, 255, 255},   {"magenta", 255, 0, 255, 255}, {"gray", 128, 128, 128, 255},
            {"grey", 128, 128, 128, 255}, {"orange", 255, 165, 0, 255},  {"purple", 128, 0, 128, 255},
            {"pink", 255, 192, 203, 255}, {"brown", 165, 42, 42, 255},   {"lime", 0, 255, 0, 255},
            {"navy", 0, 0, 128, 255},     {"teal", 0, 128, 128, 255},    {"silver", 192, 192, 192, 255},
            {"maroon", 128, 0, 0, 255},   {"olive", 128, 128, 0, 255},
    }};

    /**
     * @brief Expands a 4-bit nibble to an 8-bit replicated value.
     *
     * @param value A nibble in [0, 15].
     * @return The expanded byte, for example 0xA -> 0xAA.
     */
    [[nodiscard]] static constexpr std::uint8_t ExpandNibble(std::uint8_t value) noexcept {
        return static_cast<std::uint8_t>((value << 4U) | value);
    }

    /**
     * @brief Clamps an integral value to the 8-bit byte range.
     *
     * @tparam T Any integral type.
     * @param value Input integral value.
     * @return The clamped byte in [0, 255].
     */
    template<std::integral T>
    [[nodiscard]] static constexpr std::uint8_t ClampToByte(T value) noexcept {
        if (value < static_cast<T>(0)) {
            return 0;
        }
        if (value > static_cast<T>(255)) {
            return 255;
        }
        return static_cast<std::uint8_t>(value);
    }

    /**
     * @brief Converts a normalized float to an 8-bit byte.
     *
     * @param value Input value, conceptually in [0.0, 1.0].
     * @return A byte obtained by clamping to [0.0, 1.0], multiplying by 255,
     * and rounding to the nearest integer.
     */
    [[nodiscard]] static std::uint8_t Float01ToByte(float value) noexcept {
        const float clamped = Clamp01(value);
        return static_cast<std::uint8_t>(std::lround(clamped * 255.0f));
    }

    /**
     * @brief Converts an 8-bit byte to a normalized float.
     *
     * @param value Byte channel value in [0, 255].
     * @return The normalized floating-point value in [0.0, 1.0].
     */
    [[nodiscard]] static float ByteToFloat01(std::uint8_t value) noexcept { return static_cast<float>(value) / 255.0f; }

    /**
     * @brief Clamps a float to the range [0.0, 1.0].
     *
     * @param value Input value.
     * @return The clamped result.
     */
    [[nodiscard]] static constexpr float Clamp01(float value) noexcept {
        return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
    }

    /**
     * @brief Checks whether a character is ASCII whitespace.
     *
     * @param c Character to test.
     * @return @c true if @p c is one of the supported ASCII whitespace characters.
     */
    [[nodiscard]] static constexpr bool IsSpace(char c) noexcept {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
    }

    /**
     * @brief Trims ASCII whitespace from both ends of a string view.
     *
     * @param s Input string view.
     * @return A sub-view with leading and trailing ASCII whitespace removed.
     */
    [[nodiscard]] static constexpr std::string_view Trim(std::string_view s) noexcept {
        while (!s.empty() && IsSpace(s.front())) {
            s.remove_prefix(1);
        }
        while (!s.empty() && IsSpace(s.back())) {
            s.remove_suffix(1);
        }
        return s;
    }

    /**
     * @brief Converts an ASCII letter to lowercase.
     *
     * @param c Input character.
     * @return Lowercase ASCII equivalent if @p c is uppercase ASCII; otherwise @p c unchanged.
     */
    [[nodiscard]] static constexpr char ToLowerAscii(char c) noexcept {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
    }

    /**
     * @brief Compares two strings using ASCII case-insensitive matching.
     *
     * @param a First string.
     * @param b Second string.
     * @return @c true if the strings are equal ignoring ASCII letter case.
     */
    [[nodiscard]] static constexpr bool EqualsInsensitive(std::string_view a, std::string_view b) noexcept {
        if (a.size() != b.size()) {
            return false;
        }

        for (std::size_t i = 0; i < a.size(); ++i) {
            if (ToLowerAscii(a[i]) != ToLowerAscii(b[i])) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Checks whether a string starts with a prefix using ASCII case-insensitive matching.
     *
     * @param value Full string.
     * @param prefix Candidate prefix.
     * @return @c true if @p value begins with @p prefix ignoring ASCII letter case.
     */
    [[nodiscard]] static constexpr bool StartsWithInsensitive(std::string_view value,
                                                              std::string_view prefix) noexcept {
        if (value.size() < prefix.size()) {
            return false;
        }
        return EqualsInsensitive(value.substr(0, prefix.size()), prefix);
    }

    /**
     * @brief Converts a single hexadecimal character to its numeric value.
     *
     * @param c Hexadecimal digit character.
     * @return A value in [0, 15] on success, or @c std::nullopt if @p c is not a valid hex digit.
     */
    [[nodiscard]] static constexpr std::optional<std::uint8_t> HexDigit(char c) noexcept {
        if (c >= '0' && c <= '9') {
            return static_cast<std::uint8_t>(c - '0');
        }
        if (c >= 'a' && c <= 'f') {
            return static_cast<std::uint8_t>(10 + c - 'a');
        }
        if (c >= 'A' && c <= 'F') {
            return static_cast<std::uint8_t>(10 + c - 'A');
        }
        return std::nullopt;
    }

    /**
     * @brief Converts two hexadecimal characters to a byte.
     *
     * @param hi High nibble character.
     * @param lo Low nibble character.
     * @return A byte on success, or @c std::nullopt if either character is invalid.
     */
    [[nodiscard]] static constexpr std::optional<std::uint8_t> HexByte(char hi, char lo) noexcept {
        const auto h = HexDigit(hi);
        const auto l = HexDigit(lo);
        if (!h || !l) {
            return std::nullopt;
        }
        return static_cast<std::uint8_t>((*h << 4U) | *l);
    }

    /**
     * @brief Appends one byte as two hexadecimal digits to a string.
     *
     * @param out Destination string.
     * @param value Byte value to append.
     * @param uppercase If @c true, use uppercase digits; otherwise lowercase.
     */
    static void AppendHexByte(std::string& out, std::uint8_t value, bool uppercase) {
        static constexpr char kLower[] = "0123456789abcdef";
        static constexpr char kUpper[] = "0123456789ABCDEF";
        const char* digits = uppercase ? kUpper : kLower;

        out.push_back(digits[(value >> 4U) & 0x0F]);
        out.push_back(digits[value & 0x0F]);
    }

    /**
     * @brief Formats the stored alpha channel as a normalized decimal string.
     *
     * @return A decimal string suitable for use in @c rgba(...) output.
     *
     * @details
     * The function trims trailing zeros from the result of @c std::to_string
     * while keeping at least one digit after a trailing decimal point.
     */
    [[nodiscard]] std::string AlphaString() const {
        const float a = ByteToFloat01(a_);
        std::string s = std::to_string(a);

        while (!s.empty() && s.back() == '0') {
            s.pop_back();
        }
        if (!s.empty() && s.back() == '.') {
            s.push_back('0');
        }

        return s;
    }

    /**
     * @brief Parses a floating-point token without accepting arbitrary suffixes.
     *
     * @param token Input token.
     * @return Parsed float on success, or @c std::nullopt on failure.
     *
     * @details
     * Leading and trailing ASCII whitespace is removed before parsing.
     * The accepted syntax is intentionally simple:
     * - optional leading sign,
     * - digits,
     * - at most one decimal point.
     */
    [[nodiscard]] static std::optional<float> ParseFloatToken(std::string_view token) noexcept {
        token = Trim(token);
        if (token.empty()) {
            return std::nullopt;
        }

        bool seen_dot = false;
        bool seen_digit = false;
        std::size_t i = 0;

        if (token[i] == '+' || token[i] == '-') {
            ++i;
        }

        const auto is_digit = [](char c) noexcept { return c >= '0' && c <= '9'; };

        for (; i < token.size(); ++i) {
            const char c = token[i];
            if (is_digit(c)) {
                seen_digit = true;
                continue;
            }
            if (c == '.' && !seen_dot) {
                seen_dot = true;
                continue;
            }
            return std::nullopt;
        }

        if (!seen_digit) {
            return std::nullopt;
        }

        try {
            return std::stof(std::string(token));
        } catch (...) {
            return std::nullopt;
        }
    }

    /**
     * @brief Parses an RGB channel token.
     *
     * @param token Input token.
     * @return A byte channel value on success, or @c std::nullopt on failure.
     *
     * @details
     * Supported forms:
     * - integer or floating-point numeric values interpreted in [0, 255],
     * - percentage values such as @c 50% interpreted in [0%, 100%].
     *
     * Numeric values are clamped to [0, 255].
     * Percentage values are converted via normalized float logic.
     */
    [[nodiscard]] static std::optional<std::uint8_t> ParseRgbChannel(std::string_view token) noexcept {
        token = Trim(token);
        if (token.empty()) {
            return std::nullopt;
        }

        const bool is_percent = token.back() == '%';
        if (is_percent) {
            token.remove_suffix(1);
            const auto value = ParseFloatToken(token);
            if (!value) {
                return std::nullopt;
            }
            return Float01ToByte(*value / 100.0f);
        }

        const auto value = ParseFloatToken(token);
        if (!value) {
            return std::nullopt;
        }

        const float v = *value < 0.0f ? 0.0f : (*value > 255.0f ? 255.0f : *value);
        return static_cast<std::uint8_t>(std::lround(v));
    }

    /**
     * @brief Parses an alpha channel token.
     *
     * @param token Input token.
     * @return A byte alpha value on success, or @c std::nullopt on failure.
     *
     * @details
     * Supported forms:
     * - normalized numeric values such as @c 0.5,
     * - percentage values such as @c 25%.
     *
     * Values are interpreted in normalized alpha space and converted through
     * the same logic used by @ref Float01ToByte.
     */
    [[nodiscard]] static std::optional<std::uint8_t> ParseAlphaChannel(std::string_view token) noexcept {
        token = Trim(token);
        if (token.empty()) {
            return std::nullopt;
        }

        const bool is_percent = token.back() == '%';
        if (is_percent) {
            token.remove_suffix(1);
            const auto value = ParseFloatToken(token);
            if (!value) {
                return std::nullopt;
            }
            return Float01ToByte(*value / 100.0f);
        }

        const auto value = ParseFloatToken(token);
        if (!value) {
            return std::nullopt;
        }

        return Float01ToByte(*value);
    }

    /**
     * @brief Parses a CSS-like @c rgb(...) or @c rgba(...) function.
     *
     * @param text Full input text.
     * @param expect_alpha Whether an alpha component is required.
     * @return A parsed color on success, or @c std::nullopt on failure.
     *
     * @details
     * This helper expects comma-separated arguments only.
     * For @c rgb(...), exactly 3 channels are required.
     * For @c rgba(...), exactly 4 channels are required.
     */
    [[nodiscard]] static std::optional<Color> ParseRgbFunc(std::string_view text, bool expect_alpha) noexcept {
        const auto left = text.find('(');
        const auto right = text.rfind(')');
        if (left == std::string_view::npos || right == std::string_view::npos || right <= left) {
            return std::nullopt;
        }

        const std::string_view args = text.substr(left + 1, right - left - 1);
        std::array<std::string_view, 4> parts{};
        std::size_t count = 0;
        std::size_t start = 0;

        for (std::size_t i = 0; i <= args.size(); ++i) {
            if (i == args.size() || args[i] == ',') {
                if (count >= parts.size()) {
                    return std::nullopt;
                }
                parts[count++] = args.substr(start, i - start);
                start = i + 1;
            }
        }

        if ((!expect_alpha && count != 3) || (expect_alpha && count != 4)) {
            return std::nullopt;
        }

        const auto r = ParseRgbChannel(parts[0]);
        const auto g = ParseRgbChannel(parts[1]);
        const auto b = ParseRgbChannel(parts[2]);
        const auto a = expect_alpha ? ParseAlphaChannel(parts[3]) : std::optional<std::uint8_t>(255);

        if (!r || !g || !b || !a) {
            return std::nullopt;
        }

        return Color(*r, *g, *b, *a);
    }

    /**
     * @brief Looks up a named color from the built-in color table.
     *
     * @param text Input color name.
     * @return A parsed color on success, or @c std::nullopt if no name matches.
     *
     * @details
     * Matching is case-insensitive.
     */
    [[nodiscard]] static std::optional<Color> ParseNamedColor(std::string_view text) noexcept {
        const auto it = std::ranges::find_if(kNamedColors, [&](const NamedColorEntry& named) noexcept {
            return EqualsInsensitive(text, named.name);
        });

        if (it == kNamedColors.end()) {
            return std::nullopt;
        }

        return Color(it->r, it->g, it->b, it->a);
    }
};

} // namespace cse498
