/**
 * @file Color.hpp
 * @brief Provides a unified color representation and conversion utilities.
 *
 * This file defines the Color class used across the project to represent
 * RGBA colors. It supports construction from multiple formats such as
 * RGBA integers, normalized floats, hexadecimal strings, and CSS-style
 * rgb()/rgba() strings.
 *
 * The class is designed to be independent of external libraries.
 * Adapters (e.g., SDL_Color conversion) should be implemented separately.
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

    class Color {
    public:
        struct RGBA255 {
            std::uint8_t r;
            std::uint8_t g;
            std::uint8_t b;
            std::uint8_t a;
        };

        struct RGBA01 {
            float r;
            float g;
            float b;
            float a;
        };

        constexpr Color() = default;

        constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) noexcept :
            r_(r), g_(g), b_(b), a_(a) {}

        [[nodiscard]] static constexpr Color FromRGBA255(std::uint8_t r, std::uint8_t g, std::uint8_t b,
                                                         std::uint8_t a = 255) noexcept {
            return Color(r, g, b, a);
        }

        [[nodiscard]] static constexpr Color FromRGB255(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
            return Color(r, g, b, 255);
        }

        template<std::integral T>
        [[nodiscard]] static constexpr Color FromRGBIntegral(T r, T g, T b, T a = static_cast<T>(255)) noexcept {
            return Color(ClampToByte(r), ClampToByte(g), ClampToByte(b), ClampToByte(a));
        }

        template<std::floating_point T>
        [[nodiscard]] static Color FromRGBNormalized(T r, T g, T b, T a = static_cast<T>(1)) noexcept {
            return FromRGBA01(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b),
                              static_cast<float>(a));
        }

        [[nodiscard]] static Color FromRGBA01(float r, float g, float b, float a = 1.0f) noexcept {
            return Color(Float01ToByte(r), Float01ToByte(g), Float01ToByte(b), Float01ToByte(a));
        }

        [[nodiscard]] static Color FromRGB01(float r, float g, float b) noexcept { return FromRGBA01(r, g, b, 1.0f); }

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

        [[nodiscard]] constexpr std::uint8_t R() const noexcept { return r_; }
        [[nodiscard]] constexpr std::uint8_t G() const noexcept { return g_; }
        [[nodiscard]] constexpr std::uint8_t B() const noexcept { return b_; }
        [[nodiscard]] constexpr std::uint8_t A() const noexcept { return a_; }

        constexpr void SetR(std::uint8_t value) noexcept { r_ = value; }
        constexpr void SetG(std::uint8_t value) noexcept { g_ = value; }
        constexpr void SetB(std::uint8_t value) noexcept { b_ = value; }
        constexpr void SetA(std::uint8_t value) noexcept { a_ = value; }

        constexpr void SetRGBA255(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) noexcept {
            r_ = r;
            g_ = g;
            b_ = b;
            a_ = a;
        }

        void SetRGBA01(float r, float g, float b, float a = 1.0f) noexcept {
            r_ = Float01ToByte(r);
            g_ = Float01ToByte(g);
            b_ = Float01ToByte(b);
            a_ = Float01ToByte(a);
        }

        [[nodiscard]] constexpr RGBA255 ToRGBA255() const noexcept { return RGBA255{r_, g_, b_, a_}; }

        [[nodiscard]] constexpr std::array<std::uint8_t, 4> ToArray() const noexcept { return {r_, g_, b_, a_}; }

        [[nodiscard]] RGBA01 ToRGBA01() const noexcept {
            return RGBA01{ByteToFloat01(r_), ByteToFloat01(g_), ByteToFloat01(b_), ByteToFloat01(a_)};
        }

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

        [[nodiscard]] std::string ToRgbString() const {
            return "rgb(" + std::to_string(static_cast<unsigned>(r_)) + ", " +
                   std::to_string(static_cast<unsigned>(g_)) + ", " + std::to_string(static_cast<unsigned>(b_)) + ")";
        }

        [[nodiscard]] std::string ToRgbaString() const {
            return "rgba(" + std::to_string(static_cast<unsigned>(r_)) + ", " +
                   std::to_string(static_cast<unsigned>(g_)) + ", " + std::to_string(static_cast<unsigned>(b_)) + ", " +
                   AlphaString() + ")";
        }

        [[nodiscard]] constexpr bool operator==(const Color& other) const noexcept = default;

    private:
        struct NamedColorEntry {
            std::string_view name;
            std::uint8_t r;
            std::uint8_t g;
            std::uint8_t b;
            std::uint8_t a;
        };

        std::uint8_t r_ = 0;
        std::uint8_t g_ = 0;
        std::uint8_t b_ = 0;
        std::uint8_t a_ = 255;

        static constexpr std::array<NamedColorEntry, 20> kNamedColors{{
                {"black", 0, 0, 0, 255},      {"white", 255, 255, 255, 255}, {"red", 255, 0, 0, 255},
                {"green", 0, 128, 0, 255},    {"blue", 0, 0, 255, 255},      {"yellow", 255, 255, 0, 255},
                {"cyan", 0, 255, 255, 255},   {"magenta", 255, 0, 255, 255}, {"gray", 128, 128, 128, 255},
                {"grey", 128, 128, 128, 255}, {"orange", 255, 165, 0, 255},  {"purple", 128, 0, 128, 255},
                {"pink", 255, 192, 203, 255}, {"brown", 165, 42, 42, 255},   {"lime", 0, 255, 0, 255},
                {"navy", 0, 0, 128, 255},     {"teal", 0, 128, 128, 255},    {"silver", 192, 192, 192, 255},
                {"maroon", 128, 0, 0, 255},   {"olive", 128, 128, 0, 255},
        }};

        [[nodiscard]] static constexpr std::uint8_t ExpandNibble(std::uint8_t value) noexcept {
            return static_cast<std::uint8_t>((value << 4U) | value);
        }

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

        [[nodiscard]] static std::uint8_t Float01ToByte(float value) noexcept {
            const float clamped = Clamp01(value);
            return static_cast<std::uint8_t>(std::lround(clamped * 255.0f));
        }

        [[nodiscard]] static float ByteToFloat01(std::uint8_t value) noexcept {
            return static_cast<float>(value) / 255.0f;
        }

        [[nodiscard]] static constexpr float Clamp01(float value) noexcept {
            return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
        }

        [[nodiscard]] static constexpr bool IsSpace(char c) noexcept {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
        }

        [[nodiscard]] static constexpr std::string_view Trim(std::string_view s) noexcept {
            while (!s.empty() && IsSpace(s.front())) {
                s.remove_prefix(1);
            }
            while (!s.empty() && IsSpace(s.back())) {
                s.remove_suffix(1);
            }
            return s;
        }

        [[nodiscard]] static constexpr char ToLowerAscii(char c) noexcept {
            return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
        }

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

        [[nodiscard]] static constexpr bool StartsWithInsensitive(std::string_view value,
                                                                  std::string_view prefix) noexcept {
            if (value.size() < prefix.size()) {
                return false;
            }
            return EqualsInsensitive(value.substr(0, prefix.size()), prefix);
        }

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

        [[nodiscard]] static constexpr std::optional<std::uint8_t> HexByte(char hi, char lo) noexcept {
            const auto h = HexDigit(hi);
            const auto l = HexDigit(lo);
            if (!h || !l) {
                return std::nullopt;
            }
            return static_cast<std::uint8_t>((*h << 4U) | *l);
        }

        static void AppendHexByte(std::string& out, std::uint8_t value, bool uppercase) {
            static constexpr char kLower[] = "0123456789abcdef";
            static constexpr char kUpper[] = "0123456789ABCDEF";
            const char* digits = uppercase ? kUpper : kLower;

            out.push_back(digits[(value >> 4U) & 0x0F]);
            out.push_back(digits[value & 0x0F]);
        }

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
