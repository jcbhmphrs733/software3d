#pragma once
#include <string>
#include <cstdint>
#include "math/vec2.h"

struct Texture {
    int width = 0;
    int height = 0;
    unsigned char* data = nullptr;

    // loads a .png/.jpg/.bmp from disk
    bool load(const std::string& filepath);

    // unloads the current image data
    void clear();

    // given a UV coordinate (0.0 to 1.0), returns the color at that point as a uint32_t RGBA
    uint32_t sample(float u, float v) const;

    ~Texture();
};