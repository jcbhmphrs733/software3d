#include "rasterizer.h"
#include <algorithm>
#include <cmath>

static float edgeFunction(const Vec2& a, const Vec2& b, const Vec2& p) {
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

void DrawTriangle(Framebuffer& fb,
                  Vec2 a, Vec2 b, Vec2 c,
                  float za, float zb, float zc,
                  uint32_t color,
                  float lia, float lib, float lic, float ambientStrength,
                  const Texture* tex,
                  Vec2 uva, Vec2 uvb, Vec2 uvc,
                  float wa, float wb, float wc) {
    int minX = (int)std::max(0.0f,                      std::min({a.x, b.x, c.x}));
    int minY = (int)std::max(0.0f,                      std::min({a.y, b.y, c.y}));
    int maxX = (int)std::min((float)fb.getWidth()  - 1, std::max({a.x, b.x, c.x}));
    int maxY = (int)std::min((float)fb.getHeight() - 1, std::max({a.y, b.y, c.y}));

    float areaInv = 1.0f / edgeFunction(a, b, c);

    // Hoist per-triangle reciprocals and perspective-divided UVs out of the pixel loop
    float inv_wa = 1.0f / wa;
    float inv_wb = 1.0f / wb;
    float inv_wc = 1.0f / wc;
    Vec2 uva_w = Vec2(uva.x * inv_wa, uva.y * inv_wa);
    Vec2 uvb_w = Vec2(uvb.x * inv_wb, uvb.y * inv_wb);
    Vec2 uvc_w = Vec2(uvc.x * inv_wc, uvc.y * inv_wc);

    // Pre-compute per-vertex shade values for Gouraud interpolation
    float shadeA = ambientStrength + (1.0f - ambientStrength) * lia;
    float shadeB = ambientStrength + (1.0f - ambientStrength) * lib;
    float shadeC = ambientStrength + (1.0f - ambientStrength) * lic;

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            Vec2 p((float)x, (float)y);

            float w0 = edgeFunction(b, c, p);
            float w1 = edgeFunction(c, a, p);
            float w2 = edgeFunction(a, b, p);

            float bar0 = w0 * areaInv;
            float bar1 = w1 * areaInv;
            float bar2 = w2 * areaInv;

            if (bar0 >= 0.0f && bar1 >= 0.0f && bar2 >= 0.0f) {
                float depth = bar0 * za + bar1 * zb + bar2 * zc;

                // Gouraud: interpolate shade across the triangle
                float shade = bar0 * shadeA + bar1 * shadeB + bar2 * shadeC;
                if (shade > 1.0f) shade = 1.0f;

                uint32_t shadedColor;
                if (tex) {
                    float inv_w_interp = bar0 * inv_wa + bar1 * inv_wb + bar2 * inv_wc;

                    float u_interp = bar0 * uva_w.x + bar1 * uvb_w.x + bar2 * uvc_w.x;
                    float v_interp = bar0 * uva_w.y + bar1 * uvb_w.y + bar2 * uvc_w.y;

                    float final_u = u_interp / inv_w_interp;
                    float final_v = v_interp / inv_w_interp;

                    uint32_t texel = tex->sample(final_u, final_v);
                    
                    // Fixed: Cast to uint32_t instead of unsigned char
                    uint32_t tr = (uint32_t)(((texel >> 24) & 0xFF) * shade);
                    uint32_t tg = (uint32_t)(((texel >> 16) & 0xFF) * shade);
                    uint32_t tb = (uint32_t)(((texel >>  8) & 0xFF) * shade);
                    shadedColor = (tr << 24) | (tg << 16) | (tb << 8) | 0xFF;
                } else {
                    // Fixed: Cast to uint32_t instead of unsigned char
                    uint32_t r = (uint32_t)(((color >> 24) & 0xFF) * shade);
                    uint32_t g = (uint32_t)(((color >> 16) & 0xFF) * shade);
                    uint32_t b = (uint32_t)(((color >>  8) & 0xFF) * shade);
                    shadedColor = (r << 24) | (g << 16) | (b << 8) | 0xFF;
                }

                fb.setPixelDepth(x, y, depth, shadedColor);
            }
        }
    }
}

void DrawLine(Framebuffer& fb, Vec2 a, Vec2 b, float za, float zb, uint32_t color) {
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
        fb.setPixelDepth(x0, y0, depth - 0.00005f, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
        step++;
    }
}