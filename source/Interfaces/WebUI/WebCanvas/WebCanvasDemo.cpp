/**
 * @file WebCanvasDemo.cpp
 * @brief Demo entry point for the WebCanvas WebUI component.
 *
 * Section A — Immediate-mode primitives (pulsing circle, rotating triangle,
 * sine-wave points, moving thick line).
 *
 * Section B — ICanvasElement render pipeline using the REAL WebImage,
 * WebButton, and WebTextbox classes. Each object is constructed, configured,
 * and handed to canvas.AddElement(). canvas.RenderFrame() then dispatches
 * their Draw() calls automatically each frame in z-index order.
 *
 * In non-Emscripten builds a stub main() is provided so the file compiles.
 */

#include "../../../tools/Color.hpp"
#include "WebCanvas.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include "../WebButton/WebButton.hpp"
#include "../WebImage/WebImage.hpp"
#include "../WebTextbox/WebTextbox.hpp"
#endif

#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#ifdef __EMSCRIPTEN__

static cse498::WebCanvas* g_canvas = nullptr;

namespace {
    constexpr std::string kBgColor = "#111318";
    constexpr std::string kAxisColor = "#2a2f3a";
    constexpr std::string kHeaderRectColor = "#1e2230";
    constexpr std::string kSectionATextColor = "#6a8faf";
    constexpr std::string kPulseStrokeColor = "#00d1ff";
    constexpr std::string kPulseFillColor = "#002733";
    constexpr std::string kWhiteColor = "#ffffff";
    constexpr std::string kTriangleStrokeColor = "#ffd166";
    constexpr std::string kTriangleFillColor = "#3a2b12";
    constexpr std::string kSineTrailColor = "#7bf1a8";
    constexpr std::string kMovingLineColor = "#ff4d6d";
    constexpr std::string kSectionBTextColor = "#af8f6a";
    constexpr std::string kImagePlaceholderA = "#1a5276";
    constexpr std::string kImagePlaceholderB = "#145a32";
    constexpr std::string kImagePlaceholderC = "#6e2f1a";
    constexpr std::string kButtonStartColor = "#1e8449";
    constexpr std::string kButtonPauseColor = "#1a5276";
    constexpr std::string kScoreTextColor = "#ffd166";
    constexpr std::string kLevelTextColor = "#7bf1a8";
    constexpr std::string kStatusTextColor = "#aaaaaa";
    constexpr std::string kCanvasBorderColor = "#444444";

    constexpr std::array<std::string, 21> kHexColors{
            kBgColor,          kAxisColor,         kHeaderRectColor,     kSectionATextColor, kPulseStrokeColor,
            kPulseFillColor,   kWhiteColor,        kTriangleStrokeColor, kTriangleFillColor, kSineTrailColor,
            kMovingLineColor,  kSectionBTextColor, kImagePlaceholderA,   kImagePlaceholderB, kImagePlaceholderC,
            kButtonStartColor, kButtonPauseColor,  kScoreTextColor,      kLevelTextColor,    kStatusTextColor,
            kCanvasBorderColor};

    constexpr bool AllDemoColorsAreValid() {
        for (const auto color: kHexColors) {
            if (!cse498::Color::FromHex(color).has_value()) {
                return false;
            }
        }
        return true;
    }
    static_assert(AllDemoColorsAreValid());
} // namespace

/// @brief Per-frame callback: Section A immediate-mode, then Section B via RenderFrame().
static void demo_frame(void* /*arg*/) {
    const double now_ms = emscripten_get_now();
    const float t = static_cast<float>(now_ms * 0.001f);

    const float W = 900.0f;
    const float H = 600.0f;

    // ---- Section A: immediate-mode primitives ----

    g_canvas->Clear(kBgColor);

    g_canvas->DrawLine(0, H * 0.5f, W, H * 0.5f, 1.0f, kAxisColor);
    g_canvas->DrawLine(W * 0.5f, 0, W * 0.5f, H, 1.0f, kAxisColor);

    g_canvas->DrawRect(10.0f, 4.0f, 260.0f, 22.0f, kHeaderRectColor);
    g_canvas->DrawText(14.0f, 20.0f, "Section A: immediate-mode primitives", kSectionATextColor, 13.0f, "sans-serif");
    // Pulsing circle
    {
        const float r = 20.0f + 10.0f * std::sin(t * 2.0f);
        g_canvas->DrawCircle(140.0f, 120.0f, r, kPulseStrokeColor, 3.0f, kPulseFillColor);
        g_canvas->DrawPoint(140.0f, 120.0f, 3.0f, kWhiteColor);
    }

    // Rotating triangle
    {
        const float cx = 320.0f, cy = 140.0f, R = 55.0f;
        std::vector<cse498::WebCanvas::Vec2> tri(3);
        for (int i = 0; i < 3; ++i) {
            const float a = t + i * 2.0943951f;
            tri[i] = {cx + R * std::cos(a), cy + R * std::sin(a)};
        }
        g_canvas->DrawPolygon(tri, kTriangleStrokeColor, 3.0f, kTriangleFillColor);
    }

    // Sine-wave point trail
    for (float x = 40.0f; x <= W - 40.0f; x += 10.0f) {
        const float y = H * 0.72f + std::sin(t * 2.0f + x * 0.02f) * 55.0f;
        g_canvas->DrawPoint(x, y, 2.2f, kSineTrailColor);
    }

    // Moving thick line
    {
        const float x1 = 60.0f, y1 = 420.0f + std::sin(t * 1.5f) * 30.0f;
        const float x2 = 420.0f, y2 = 520.0f + std::cos(t * 1.3f) * 30.0f;
        g_canvas->DrawLine(x1, y1, x2, y2, 6.0f, kMovingLineColor);
    }

    // ---- Section B: real WebImage / WebButton / WebTextbox via ICanvasElement ----

    g_canvas->DrawRect(500.0f, 4.0f, 390.0f, 22.0f, kHeaderRectColor);
    g_canvas->DrawText(504.0f, 20.0f, "Section B: real WebImage / WebButton / WebTextbox", kSectionBTextColor, 13.0f,
                       "sans-serif");

    g_canvas->RenderFrame();
}

/// @brief Entry point: constructs real WebImage, WebButton, WebTextbox objects,
///        registers them with the canvas, and starts the animation loop.
int main() {
    static cse498::WebCanvas canvas("web-canvas");
    g_canvas = &canvas;

    EM_ASM(
            {
                let c = document.getElementById("web-canvas");
                if (!c) {
                    c = document.createElement("canvas");
                    c.id = "web-canvas";
                    document.body.appendChild(c);
                }
                c.width = 900;
                c.height = 600;
                c.style.border = "1px solid " + UTF8ToString($0);
                c.style.display = "block";
                c.style.margin = "16px auto";
            },
            kCanvasBorderColor.c_str());

    // ---- WebImage (z = 0) ----
    // Empty src triggers the error path → Draw() renders a BlankRect placeholder
    // on the canvas, proving the error-mode + canvas rendering path works.

    auto img_a = std::make_unique<cse498::WebImage>("");
    img_a->SetSize(80, 80);
    img_a->SetPlaceholderColor(kImagePlaceholderA);
    img_a->SetCanvasRect(520.0f, 50.0f, 80.0f, 80.0f);
    img_a->SetZIndex(0);
    img_a->Hide(); // DOM <img> hidden; only the canvas rendering is shown

    auto img_b = std::make_unique<cse498::WebImage>("");
    img_b->SetSize(80, 80);
    img_b->SetPlaceholderColor(kImagePlaceholderB);
    img_b->SetCanvasRect(620.0f, 50.0f, 80.0f, 80.0f);
    img_b->SetZIndex(0);
    img_b->Hide();

    auto img_c = std::make_unique<cse498::WebImage>("");
    img_c->SetSize(80, 80);
    img_c->SetPlaceholderColor(kImagePlaceholderC);
    img_c->SetCanvasRect(720.0f, 50.0f, 80.0f, 80.0f);
    img_c->SetZIndex(0);
    img_c->Hide();

    // ---- WebButton (z = 1) ----
    // Enabled and disabled buttons: Draw() renders rect + label on canvas.

    auto btn_start = std::make_unique<cse498::WebButton>("Start Game");
    btn_start->SetSize(200, 28);
    btn_start->SetBackgroundColor(kButtonStartColor);
    btn_start->SetTextColor(kWhiteColor);
    btn_start->SetCanvasRect(520.0f, 160.0f, 200.0f, 28.0f);
    btn_start->SetZIndex(1);
    btn_start->Hide(); // DOM <button> hidden; canvas rendering is shown

    auto btn_pause = std::make_unique<cse498::WebButton>("Pause");
    btn_pause->SetSize(200, 28);
    btn_pause->SetBackgroundColor(kButtonPauseColor);
    btn_pause->SetTextColor(kWhiteColor);
    btn_pause->Disable(); // disabled → Draw() uses grey background
    btn_pause->SetCanvasRect(730.0f, 160.0f, 150.0f, 28.0f);
    btn_pause->SetZIndex(1);
    btn_pause->Hide();

    // ---- WebTextbox (z = 2) ----
    // WebTextbox does not append to DOM body in its constructor,
    // so no Hide() needed; the text is rendered only on canvas.

    auto tb_score = std::make_unique<cse498::WebTextbox>("Score: 99999");
    tb_score->SetFontSize(15.0f);
    tb_score->SetColor(kScoreTextColor);
    tb_score->SetCanvasPosition(520.0f, 220.0f);
    tb_score->SetZIndex(2);

    auto tb_level = std::make_unique<cse498::WebTextbox>("Level: 42");
    tb_level->SetFontSize(15.0f);
    tb_level->SetColor(kLevelTextColor);
    tb_level->SetCanvasPosition(520.0f, 245.0f);
    tb_level->SetZIndex(2);

    auto tb_status = std::make_unique<cse498::WebTextbox>("Status: READY");
    tb_status->SetFontSize(13.0f);
    tb_status->SetColor(kStatusTextColor);
    tb_status->SetCanvasPosition(520.0f, 270.0f);
    tb_status->SetZIndex(2);

    // ---- invisible element (z = 3) — skipped by RenderFrame() ----
    auto invisible = std::make_unique<cse498::WebTextbox>("THIS SHOULD NOT APPEAR");
    invisible->SetCanvasPosition(520.0f, 310.0f);
    invisible->SetZIndex(3);
    invisible->SetVisible(false); // ICanvasElement::SetVisible — skipped by RenderFrame

    canvas.AddElement(std::move(img_a));
    canvas.AddElement(std::move(img_b));
    canvas.AddElement(std::move(img_c));
    canvas.AddElement(std::move(btn_start));
    canvas.AddElement(std::move(btn_pause));
    canvas.AddElement(std::move(tb_score));
    canvas.AddElement(std::move(tb_level));
    canvas.AddElement(std::move(tb_status));
    canvas.AddElement(std::move(invisible));

    emscripten_set_main_loop_arg(demo_frame, nullptr, 0, 1);
    return 0;
}

#else
/// @brief Non-Emscripten stub: provided so the file compiles in native builds.
int main() { return 0; }
#endif
