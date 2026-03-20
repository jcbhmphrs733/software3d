#pragma once
#include "framebuffer.h"
#include "math/vec2.h"
#include <cstdint>

// fills a triangle with depth testing and depth-based darkening.
// depthMin/depthMax are the scene's depth range this frame — used to normalize
// the interpolated depth so shading spans full contrast regardless of frustum size.
// depthFalloff controls how quickly pixels darken with distance: 0 = flat color.
void DrawTriangle(Framebuffer& fb,
                  Vec2 a, Vec2 b, Vec2 c,
                  float za, float zb, float zc,
                  uint32_t color, float depthFalloff,
                  float depthMin, float depthMax);

// draws a line between two screen positions with depth testing
void DrawLine(Framebuffer& fb, Vec2 a, Vec2 b, float za, float zb, uint32_t color);
