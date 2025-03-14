#pragma once
#include <vector>
#include <iostream>
#include <Eigen/Dense>
#include <algorithm>

#include "Color.h"
#include "MathUtils.h"
#include "GraphicsSettings.h"
#include "GraphicsType.h"

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

    void drawImage(uint32_t *image, int imageWidth, int imageHeight, int width, int height);
    void drawImage(Eigen::Vector4f *image, int imageWidth, int imageHeight, int width, int height);

    T* data() { return frameBuffer.data(); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    int width;
    int height;
    int size;
    std::vector<T> frameBuffer;
};

template<typename T>
Buffer<T>::Buffer(int width, int height)
{
    this->width = width;
    this->height = height;
    size = width * height;
    this->frameBuffer.resize(size);
}

template<typename T>
void Buffer<T>::clear(T color)
{
    std::fill(frameBuffer.begin(), frameBuffer.end(), color);
}

template<typename T>
T& Buffer<T>::referPixel(int x, int y)
{
    y = height - y - 1;
    return frameBuffer[std::clamp(y * width + x, 0, size - 1)];
}

template<typename T>
T Buffer<T>::getPixel(int x, int y)
{
    y = height - y - 1;
    return frameBuffer[std::clamp(y * width + x, 0, size - 1)];
}

template<typename T>
void Buffer<T>::putPixel(int x, int y, T color)
{
    y = height - y - 1;
    frameBuffer[std::clamp(y * width + x, 0, size - 1)] = color;
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

template<typename T>
void Buffer<T>::drawImage(uint32_t* image, int imageWidth, int imageHeight, int width, int height)
{
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            int rx = static_cast<int>(static_cast<float>(x) * imageWidth / width);
            int ry = static_cast<int>(static_cast<float>(y) * imageHeight / height);

            putPixel(x, this->height - 1 - y, image[ry * imageWidth + rx]);
        }
    }
}

template<typename T>
void Buffer<T>::drawImage(Eigen::Vector4f* image, int imageWidth, int imageHeight, int width, int height)
{
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            int rx = static_cast<int>(static_cast<float>(x) * imageWidth / width);
            int ry = static_cast<int>(static_cast<float>(y) * imageHeight / height);

            putPixel(x, this->height - 1 - y, Color::Make(image[ry * imageWidth + rx]));
        }
    }
}

class Colorbuffer : public Buffer<uint32_t> {
public:
    Colorbuffer(int width, int height) : Buffer<uint32_t>(width, height) {}
};

template class Buffer<float>;
template class Buffer<uint32_t>;

class Framebuffer {
public:
    Framebuffer(int width, int height) 
        : colorBuffer(width, height),
        pixelBuffer(width, height)
    {
        this->width = width;
        this->height = height;
    }

    Colorbuffer colorBuffer;
    Buffer<PixelData> pixelBuffer;
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    int width;
    int height;
};
