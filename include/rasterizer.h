#pragma once
#include "framebuffer.h"
#include "math/vec2.h"
#include <cstdint>

// fills a triangle with depth testing — za/zb/zc are the NDC z values [0,1] per vertex
void DrawTriangle(Framebuffer& fb,
                  Vec2 a, Vec2 b, Vec2 c,
                  float za, float zb, float zc,
                  uint32_t color);

// draws a line between two screen positions (no depth test — always on top)
void DrawLine(Framebuffer& fb, Vec2 a, Vec2 b, uint32_t color);
