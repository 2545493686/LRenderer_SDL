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
    T& referPixel(int x, int y);

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

template<typename T>
Buffer<T>::Buffer(int width, int height)
{
    this->width = width;
    this->height = height;
    this->frameBuffer.resize(width * height);
}

template<typename T>
void Buffer<T>::clear(T color)
{
    std::fill(frameBuffer.begin(), frameBuffer.end(), color);
}

template<typename T>
T& Buffer<T>::referPixel(int x, int y)
{
    return frameBuffer[y * width + x];
}

template<typename T>
T Buffer<T>::getPixel(int x, int y)
{
    return frameBuffer[y * width + x];
}

template<typename T>
void Buffer<T>::putPixel(int x, int y, T color)
{
    if (x >= 0 && x < width && y >= 0 && y < height) {
        frameBuffer[y * width + x] = color;
    }
}

// DDA算法绘制直线 TODO：Bresenham
template<typename T>
void Buffer<T>::drawLine(int x0, int y0, int x1, int y1, T color)
{
    float k = (y1 - y0) / (float)(x1 - x0);
    if (k >= -1 && k <= 1) {
        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        float y = y0;
        for (int x = x0; x <= x1; ++x) {
            putPixel(x, y, color);
            y += k;
        }
    }
    else {
        if (y0 > y1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        k = 1 / k;
        float x = x0;
        for (int y = y0; y <= y1; ++y) {
            putPixel(x, y, color);
            x += k;
        }
    }
}

template<typename T>
void Buffer<T>::drawLine(Eigen::Vector2f p0, Eigen::Vector2f p1, T color)
{
    drawLine(static_cast<int>(p0.x()), static_cast<int>(p0.y()), static_cast<int>(p1.x()), static_cast<int>(p1.y()), color);
}


class Framebuffer : public Buffer<uint32_t> {
public:
    Framebuffer(int width, int height) : Buffer<uint32_t>(width, height) {}
};

template class Buffer<float>;
template class Buffer<uint32_t>;
