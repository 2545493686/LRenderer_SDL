#pragma once
#include <vector>

class Framebuffer {
public:
    Framebuffer(int width, int height);

    void clear(uint32_t color);
    void putPixel(int x, int y, uint32_t color);
    uint32_t* data() { return frameBuffer.data(); }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    int width;
    int height;
    std::vector<uint32_t> frameBuffer;
};
