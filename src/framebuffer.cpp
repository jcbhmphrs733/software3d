#include "framebuffer.h" // resolved from include/ via CMakeLists.txt target_include_directories

// Framebuffer.cpp
// Implements a simple software framebuffer that stores pixel data in CPU memory.
// The framebuffer is cleared and individual pixels can be set. The pixel data can then be uploaded to an OpenGL texture for display.
Framebuffer::Framebuffer(int w, int h) : width(w), height(h) {
    pixels = new unsigned char[width * height * 4];
    depth  = new float[width * height];
    clear(0x000000FF);
    clearDepth();
}

Framebuffer::~Framebuffer() {
    delete[] pixels;
    delete[] depth;
}

void Framebuffer::clear(uint32_t color) {
    unsigned char r = (color >> 24) & 0xFF;
    unsigned char g = (color >> 16) & 0xFF;
    unsigned char b = (color >> 8) & 0xFF;
    unsigned char a = color & 0xFF;

    for (int i = 0; i < width * height; ++i) {
        pixels[i * 4 + 0] = r;
        pixels[i * 4 + 1] = g;
        pixels[i * 4 + 2] = b;
        pixels[i * 4 + 3] = a;
    } 
    // reason why its 4 is because we are using RGBA format and each pixel needs 4 bytes to store the color information (red, green, blue, alpha)
}

void Framebuffer::clearDepth() {
    for (int i = 0; i < width * height; ++i) {
        depth[i] = 1.0f; // 1.0 = maximum depth (far plane)
    }
}

bool Framebuffer::setPixelDepth(int x, int y, float d, uint32_t color) {
    if (x < 0 || x >= width || y < 0 || y >= height) return false;

    int i = y * width + x;
    if (d >= depth[i]) return false; // already something closer here

    depth[i] = d; // update depth
    int index = i * 4;
    pixels[index + 0] = (color >> 24) & 0xFF;
    pixels[index + 1] = (color >> 16) & 0xFF;
    pixels[index + 2] = (color >> 8)  & 0xFF;
    pixels[index + 3] =  color        & 0xFF;
    return true;
}

void Framebuffer::setPixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    int index = (y * width + x) * 4;
    pixels[index + 0] = (color >> 24) & 0xFF;
    pixels[index + 1] = (color >> 16) & 0xFF;
    pixels[index + 2] = (color >> 8) & 0xFF;
    pixels[index + 3] = color & 0xFF;
}

const unsigned char* Framebuffer::getPixels() const {
    return pixels;
}

int Framebuffer::getWidth() const {
    return width;
}

int Framebuffer::getHeight() const {
    return height;
}