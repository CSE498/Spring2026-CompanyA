// WebCanvas.js
// Emscripten JS library glue for WebCanvas primitive drawing (Canvas2D).
//
// Link with:
//   --js-library group_specific_content/Group-18/WebUI/WebCanvas/WebCanvas.js
//
// Design notes:
// - Works in browser: uses native Canvas2D API.
// - Safe in Node/no-DOM env: early return BEFORE touching UTF8ToString/DOM.
// - Avoids $-prefixed internal helpers to prevent missing-symbol issues across emsdk versions.

mergeInto(LibraryManager.library, {
    webcanvas__clear: function (idPtr, cssColorPtr) {
        // "helper" logic inlined for robustness
        if (typeof document === "undefined") return;

        var id = UTF8ToString(idPtr);
        var canvas = document.getElementById(id);
        if (!canvas) return;

        var ctx = canvas.getContext && canvas.getContext("2d");
        if (!ctx) return;

        var color = UTF8ToString(cssColorPtr);

        ctx.save();
        ctx.fillStyle = color;
        ctx.fillRect(0, 0, canvas.width, canvas.height);
        ctx.restore();
    },

    webcanvas__draw_line: function (idPtr, x1, y1, x2, y2, lineWidth, strokeColorPtr) {
        if (typeof document === "undefined") return;

        var id = UTF8ToString(idPtr);
        var canvas = document.getElementById(id);
        if (!canvas) return;

        var ctx = canvas.getContext && canvas.getContext("2d");
        if (!ctx) return;

        var stroke = UTF8ToString(strokeColorPtr);

        ctx.save();
        ctx.beginPath();
        ctx.lineWidth = lineWidth;
        ctx.strokeStyle = stroke;
        ctx.moveTo(x1, y1);
        ctx.lineTo(x2, y2);
        ctx.stroke();
        ctx.restore();
    },

    webcanvas__draw_circle: function (idPtr, cx, cy, r, strokeColorPtr, lineWidth, fillColorPtr) {
        if (typeof document === "undefined") return;

        var id = UTF8ToString(idPtr);
        var canvas = document.getElementById(id);
        if (!canvas) return;

        var ctx = canvas.getContext && canvas.getContext("2d");
        if (!ctx) return;

        var stroke = UTF8ToString(strokeColorPtr);
        var fill = UTF8ToString(fillColorPtr); // may be ""

        ctx.save();
        ctx.beginPath();
        ctx.arc(cx, cy, r, 0, Math.PI * 2);

        if (fill && fill.length > 0) {
            ctx.fillStyle = fill;
            ctx.fill();
        }

        if (lineWidth > 0) {
            ctx.lineWidth = lineWidth;
            ctx.strokeStyle = stroke;
            ctx.stroke();
        }

        ctx.restore();
    },

    webcanvas__draw_polygon: function (idPtr, coordsXYPtr, count, strokeColorPtr, lineWidth, fillColorPtr) {
        if (typeof document === "undefined") return;

        var id = UTF8ToString(idPtr);
        var canvas = document.getElementById(id);
        if (!canvas) return;

        var ctx = canvas.getContext && canvas.getContext("2d");
        if (!ctx) return;

        if (!coordsXYPtr || count <= 1) return;

        var stroke = UTF8ToString(strokeColorPtr);
        var fill = UTF8ToString(fillColorPtr);

        // coordsXYPtr points to float array: [x0,y0,x1,y1,...]
        var base = coordsXYPtr >> 2; // float index in HEAPF32
        var H = HEAPF32;

        ctx.save();
        ctx.beginPath();
        ctx.moveTo(H[base + 0], H[base + 1]);

        for (var i = 1; i < count; i++) {
            ctx.lineTo(H[base + i * 2 + 0], H[base + i * 2 + 1]);
        }

        ctx.closePath();

        if (fill && fill.length > 0) {
            ctx.fillStyle = fill;
            ctx.fill();
        }

        if (lineWidth > 0) {
            ctx.lineWidth = lineWidth;
            ctx.strokeStyle = stroke;
            ctx.stroke();
        }

        ctx.restore();
    }
});

// Tell emcc we need UTF8ToString available in the library scope.
webcanvas__clear__deps = ['$UTF8ToString'];
webcanvas__draw_line__deps = ['$UTF8ToString'];
webcanvas__draw_circle__deps = ['$UTF8ToString'];
webcanvas__draw_polygon__deps = ['$UTF8ToString'];