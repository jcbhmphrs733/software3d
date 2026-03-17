#include "rasterizer.h"
#include <algorithm>
#include <cmath>

static float edgeFunction(const Vec2& a, const Vec2& b, const Vec2& p) {
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

void DrawTriangle(Framebuffer& fb,
                  Vec2 a, Vec2 b, Vec2 c,
                  float za, float zb, float zc,
                  uint32_t color) {
    int minX = (int)std::max(0.0f,                      std::min({a.x, b.x, c.x}));
    int minY = (int)std::max(0.0f,                      std::min({a.y, b.y, c.y}));
    int maxX = (int)std::min((float)fb.getWidth()  - 1, std::max({a.x, b.x, c.x}));
    int maxY = (int)std::min((float)fb.getHeight() - 1, std::max({a.y, b.y, c.y}));

    float areaInv = 1.0f / edgeFunction(a, b, c); // total triangle area (reciprocal for speed)

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            Vec2 p((float)x, (float)y);

            float w0 = edgeFunction(b, c, p);
            float w1 = edgeFunction(c, a, p);
            float w2 = edgeFunction(a, b, p);

            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                // barycentric coordinates — how much each vertex contributes to this pixel
                float bar0 = w0 * areaInv;
                float bar1 = w1 * areaInv;
                float bar2 = w2 * areaInv;

                // interpolate depth across the triangle
                float depth = bar0 * za + bar1 * zb + bar2 * zc;

                fb.setPixelDepth(x, y, depth, color);
            }
        }
    }
}

void DrawLine(Framebuffer& fb, Vec2 a, Vec2 b, uint32_t color) {
    // Bresenham's line algorithm
    int x0 = (int)a.x, y0 = (int)a.y;
    int x1 = (int)b.x, y1 = (int)b.y;

    int dx =  std::abs(x1 - x0);
    int dy = -std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        fb.setPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}
