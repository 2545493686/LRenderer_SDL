#pragma once
// deepseek 生成 未验证
#define _USE_MATH_DEFINES

#include <Eigen/Dense>
#include <vector>
#include <algorithm>

class LatitudeLongitudeMap
{
public:
    enum class SampleType
    {
        Direct,
        Bilinear
    };

    explicit LatitudeLongitudeMap(int width, int height)
        : width(width), height(height)
    {
        data = new Eigen::Vector4f[width * height];
    }

    ~LatitudeLongitudeMap()
    {
        delete[] data;
        data = nullptr;
    }

    Eigen::Vector4f *data = nullptr;
    int width = 0;
    int height = 0;

    Eigen::Vector4f Sample(const Eigen::Vector3f &direction, SampleType sampleType = SampleType::Direct) const;
    Eigen::Vector3f GetDirection(const Eigen::Vector2f &uv) const;
    
    EIGEN_ALWAYS_INLINE void PutPixel(int x, int y, const Eigen::Vector4f &color)
    {
        data[y * width + x] = color;
    }

    EIGEN_ALWAYS_INLINE void PutPixel(const Eigen::Vector2f &uv, const Eigen::Vector4f &color)
    {
        float u = std::clamp(uv.x(), 0.0f, 1.0f);
        float v = std::clamp(uv.y(), 0.0f, 1.0f);

        int x0 = static_cast<int>(u * width);
        int y0 = static_cast<int>(v * height);
        x0 = std::max(0, std::min(width - 1, x0));
        y0 = std::max(0, std::min(height - 1, y0));

        data[y0 * width + x0] = color;
    }

private:
    void DetermineUV(const Eigen::Vector3f &dir, Eigen::Vector2f &uv) const;
    Eigen::Vector4f SampleData(const Eigen::Vector2f &uv, SampleType sampleType) const;
};
