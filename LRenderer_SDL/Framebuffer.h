#pragma once
#include <vector>

#include "Eigen/Dense"
#include "Mesh.h"
#include "Color.h"

class Framebuffer {
public:
    Framebuffer(int width, int height);

    void clear(uint32_t color);
    void putPixel(int x, int y, uint32_t color);
    void drawLine(int x0, int y0, int x1, int y1, uint32_t color);
    void drawLine(Eigen::Vector2f p0, Eigen::Vector2f p1, uint32_t color);

    void drawMesh(Mesh &mesh);

    uint32_t* data() { return frameBuffer.data(); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    int width;
    int height;
    std::vector<uint32_t> frameBuffer;
};
