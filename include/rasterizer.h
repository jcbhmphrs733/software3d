#pragma once
#include "framebuffer.h"
#include "math/vec2.h"
#include <cstdint>

void DrawTriangle(Framebuffer& fb, Vec2 a, Vec2 b, Vec2 c, uint32_t color);
