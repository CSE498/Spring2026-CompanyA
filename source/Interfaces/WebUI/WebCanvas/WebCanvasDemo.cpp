#include "WebCanvas.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <cmath>
#include <vector>

#ifdef __EMSCRIPTEN__
static cse498::WebCanvas* g_canvas = nullptr;

static void demo_frame(void* /*arg*/) {
    const double now_ms = emscripten_get_now();
    const float t = static_cast<float>(now_ms * 0.001); // seconds

    // Demo canvas size (kept in sync with JS-created canvas)
    const float W = 900.0f;
    const float H = 600.0f;

    // Background
    g_canvas->Clear("#111318");

    // Axes
    g_canvas->DrawLine(0, H * 0.5f, W, H * 0.5f, 1.0f, "#2a2f3a");
    g_canvas->DrawLine(W * 0.5f, 0, W * 0.5f, H, 1.0f, "#2a2f3a");

    // Pulsing circle
    {
        const float r = 20.0f + 10.0f * std::sin(t * 2.0f);
        g_canvas->DrawCircle(140.0f, 120.0f, r, "#00d1ff", 3.0f, "#002733");
        g_canvas->DrawPoint(140.0f, 120.0f, 3.0f, "#ffffff");
    }

    // Rotating triangle (polygon)
    {
        const float cx = 320.0f;
        const float cy = 140.0f;
        const float R  = 55.0f;

        std::vector<cse498::WebCanvas::Vec2> tri(3);
        for (int i = 0; i < 3; ++i) {
            const float a = t + i * 2.0943951f; // 2*pi/3
            tri[i] = { cx + R * std::cos(a), cy + R * std::sin(a) };
        }
        g_canvas->DrawPolygon(tri, "#ffd166", 3.0f, "#3a2b12");
    }

    // Sine points
    {
        for (float x = 40.0f; x <= W - 40.0f; x += 10.0f) {
            const float y = H * 0.72f + std::sin(t * 2.0f + x * 0.02f) * 55.0f;
            g_canvas->DrawPoint(x, y, 2.2f, "#7bf1a8");
        }
    }

    // Moving thick line
    {
        const float x1 = 60.0f;
        const float y1 = 420.0f + std::sin(t * 1.5f) * 30.0f;
        const float x2 = 420.0f;
        const float y2 = 520.0f + std::cos(t * 1.3f) * 30.0f;
        g_canvas->DrawLine(x1, y1, x2, y2, 6.0f, "#ff4d6d");
    }
}

int main() {
    static cse498::WebCanvas canvas("web-canvas");
    g_canvas = &canvas;

    // Create a <canvas> if not present, and set a fixed size & basic styles.
    EM_ASM({
        let c = document.getElementById("web-canvas");
        if (!c) {
            c = document.createElement("canvas");
            c.id = "web-canvas";
            document.body.appendChild(c);
        }
        c.width = 900;
        c.height = 600;
        c.style.border = "1px solid #333";
        c.style.display = "block";
        c.style.margin = "16px auto";
    });

    // 60fps-ish
    emscripten_set_main_loop_arg(demo_frame, nullptr, 0, 1);
    return 0;
}
#else
// Non-emscripten build: provide a tiny stub main so the file can still compile if included.
int main() { return 0; }
#endif