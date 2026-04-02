#pragma once
#include <cstdint>
#include <limits>

// framebuffer.h
// This is a declaration of the framebuffer class, it defines the interface without any implementation details
class Framebuffer {
public:
    Framebuffer(int w, int h);
    ~Framebuffer();

    void clear(uint32_t color);          // clear color buffer
    void blitBackground(const unsigned char* src); // copy a pre-scaled RGBA image into the color buffer
    void clearDepth();                   // reset depth buffer to max depth (1.0f)

    // only writes color if depth is closer than what's already stored
    bool setPixelDepth(int x, int y, float depth, uint32_t color);
    void setPixel(int x, int y, uint32_t color); // unconditional write (for lines)

    const unsigned char* getPixels() const;
    int getWidth() const;
    int getHeight() const;

private:
    int width;
    int height;
    unsigned char* pixels;
    float* depth;  // one float per pixel, 0.0 = near, 1.0 = far
};