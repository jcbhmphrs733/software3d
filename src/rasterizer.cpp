#include "rasterizer.h"
#include <algorithm>
#include <cmath>

static float edgeFunction(const Vec2& a, const Vec2& b, const Vec2& p) {
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

void DrawTriangle(Framebuffer& fb,
                  Vec2 a, Vec2 b, Vec2 c,
                  float za, float zb, float zc,
                  uint32_t color, float depthFalloff,
                  float depthMin, float depthMax) {
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

            // Normalize by area so the sign is consistent regardless of winding order.
            // Front-facing triangles in our Y-down screen space have negative area,
            // so w0/w1/w2 are also negative for interior pixels — checking the raw
            // values against >= 0 would always fail.  Multiplying by areaInv flips
            // them to positive, making this test winding-agnostic.
            float bar0 = w0 * areaInv;
            float bar1 = w1 * areaInv;
            float bar2 = w2 * areaInv;

            if (bar0 >= 0.0f && bar1 >= 0.0f && bar2 >= 0.0f) {
                float depth = bar0 * za + bar1 * zb + bar2 * zc;

                // normalize depth to [0,1] within the scene's actual depth range
                // so shading has full contrast regardless of frustum size
                float range = depthMax - depthMin;
                float normalizedDepth = (range > 0.0f) ? (depth - depthMin) / range : 0.0f;

                float shade = 1.0f - normalizedDepth * depthFalloff;
                if (shade < 0.0f) shade = 0.0f;
                unsigned char r = (unsigned char)(((color >> 24) & 0xFF) * shade);
                unsigned char g = (unsigned char)(((color >> 16) & 0xFF) * shade);
                unsigned char b = (unsigned char)(((color >>  8) & 0xFF) * shade);
                uint32_t shadedColor = (r << 24) | (g << 16) | (b << 8) | 0xFF;

                fb.setPixelDepth(x, y, depth, shadedColor);
            }
        }
    }
}

void DrawLine(Framebuffer& fb, Vec2 a, Vec2 b, float za, float zb, uint32_t color) {
    // Bresenham's line algorithm with depth interpolation
    int x0 = (int)a.x, y0 = (int)a.y;
    int x1 = (int)b.x, y1 = (int)b.y;

    int dx =  std::abs(x1 - x0);
    int dy = -std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int steps = std::max(std::abs(x1 - x0), std::abs(y1 - y0));
    int step = 0;

    while (true) {
        float t = (steps > 0) ? (float)step / (float)steps : 0.0f;
        float depth = za + t * (zb - za);
        // small bias pushes the edge just in front of the triangle fill
        // to prevent z-fighting without affecting occlusion by other triangles
        fb.setPixelDepth(x0, y0, depth - 0.00005f, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
        step++;
    }
}
