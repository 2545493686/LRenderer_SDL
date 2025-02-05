#pragma once
#include <vector>
#include <iostream>
#include "Eigen/Dense"
#include "algorithm"

#include "Mesh.h"
#include "Color.h"
#include "Shader.h"
#include "EnvVariableCreater.h"
#include "MathUtils.h"

template<typename T>
class Buffer {
public:
    Buffer(int width, int height);

    void clear(T color);
    void putPixel(int x, int y, T color);
    T getPixel(int x, int y);

    void drawLine(int x0, int y0, int x1, int y1, T color);
    void drawLine(Eigen::Vector2f p0, Eigen::Vector2f p1, T color);

    T* data() { return frameBuffer.data(); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    int width;
    int height;
    std::vector<T> frameBuffer;
};

class Framebuffer : public Buffer<uint32_t> {
public:
    Framebuffer(int width, int height) : Buffer<uint32_t>(width, height) {}
};

template class Buffer<float>;
template class Buffer<uint32_t>;
