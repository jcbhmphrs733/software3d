#pragma once
#include <cstdint>

// framebuffer.h
// This is a declaration of the framebuffer class, it defines the interface without any implementation details
class Framebuffer {
public:
    Framebuffer(int w, int h);
    ~Framebuffer();

    void clear(uint32_t color);
    void setPixel(int x, int y, uint32_t color);
    
    const unsigned char* getPixels() const;
    int getWidth() const;
    int getHeight() const;

private:
    int width;
    int height;
    unsigned char* pixels;
};