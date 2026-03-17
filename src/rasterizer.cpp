#include "rasterizer.h"
#include <algorithm> // for std::min, std::max

// edge function: returns positive if P is to the left of edge AB, negative if to the right
static float edgeFunction(const Vec2& a, const Vec2& b, const Vec2& p) {
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

void DrawTriangle(Framebuffer& fb, Vec2 a, Vec2 b, Vec2 c, uint32_t color) {
    // compute bounding box clamped to framebuffer dimensions
    int minX = (int)std::max(0.0f,                    std::min({a.x, b.x, c.x}));
    int minY = (int)std::max(0.0f,                    std::min({a.y, b.y, c.y}));
    int maxX = (int)std::min((float)fb.getWidth()  - 1, std::max({a.x, b.x, c.x}));
    int maxY = (int)std::min((float)fb.getHeight() - 1, std::max({a.y, b.y, c.y}));

    // test every pixel in the bounding box
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            Vec2 p = Vec2((float)x, (float)y);

            float w0 = edgeFunction(b, c, p); // edge BC
            float w1 = edgeFunction(c, a, p); // edge CA
            float w2 = edgeFunction(a, b, p); // edge AB

            // point is inside if all edge functions are >= 0
            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                fb.setPixel(x, y, color);
            }
        }
    }
}
