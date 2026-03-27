#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "texture.h"
#include <iostream>

bool Texture::load(const std::string& filepath) {
    int channels;
    data = stbi_load(filepath.c_str(), &width, &height, &channels, 4); // force 4 channels (RGBA)
if (!data) {
    std::cerr << "Texture: failed to load " << filepath << "\n";
    std::cerr << "stb_image reason: " << stbi_failure_reason() << "\n";
    return false;
}
    return true;
}

uint32_t Texture::sample(float u, float v) const {
    if (!data) return 0xFF00FFFF; // magenta = missing texture

    // wrap UVs so values outside 0-1 tile instead of going out of bounds
    u = u - (int)u;
    v = v - (int)v;
    if (u < 0.0f) u += 1.0f;
    if (v < 0.0f) v += 1.0f;

    // convert UV to pixel coordinates
    int x = (int)(u * (width  - 1));
    int y = (int)((1.0f - v) * (height - 1)); // flip V because image Y is top-down

    int idx = (y * width + x) * 4;
    unsigned char r = data[idx + 0];
    unsigned char g = data[idx + 1];
    unsigned char b = data[idx + 2];
    unsigned char a = data[idx + 3];

    return (r << 24) | (g << 16) | (b << 8) | a;
}

Texture::~Texture() {
    if (data) stbi_image_free(data);
}