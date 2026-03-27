#pragma once
#include "framebuffer.h"
#include "math/vec2.h"
#include "texture.h"
#include <cstdint>

// fills a triangle with depth testing and shading.
// depthMin/depthMax normalize interpolated depth for depth-falloff mode.
// lightIntensity is a pre-computed [0,1] diffuse factor for diffuse lighting mode.
// useDiffuse selects between depth-falloff shading and diffuse lighting.
void DrawTriangle(Framebuffer& fb,
                  Vec2 a, Vec2 b, Vec2 c,
                  float za, float zb, float zc,
                  uint32_t color, float depthFalloff,
                  float depthMin, float depthMax,
                  float lightIntensity, float ambientStrength, bool useDiffuse,
                  const Texture* tex,
                  Vec2 uva, Vec2 uvb, Vec2 uvc,
                  float wa, float wb, float wc);
                  

// draws a line between two screen positions with depth testing
void DrawLine(Framebuffer& fb, Vec2 a, Vec2 b, float za, float zb, uint32_t color);