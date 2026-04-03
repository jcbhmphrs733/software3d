#pragma once
#include "framebuffer.h"
#include "math/vec2.h"
#include "texture.h"
#include <cstdint>

// fills a triangle with depth testing and Gouraud shading.
// lia/lib/lic are per-vertex diffuse light intensities, interpolated across the triangle.
void DrawTriangle(Framebuffer& fb,
                  Vec2 a, Vec2 b, Vec2 c,
                  float za, float zb, float zc,
                  uint32_t color,
                  float lia, float lib, float lic, float ambientStrength,
                  const Texture* tex,
                  Vec2 uva, Vec2 uvb, Vec2 uvc,
                  float wa, float wb, float wc);
                  

// draws a line between two screen positions with depth testing
void DrawLine(Framebuffer& fb, Vec2 a, Vec2 b, float za, float zb, uint32_t color);