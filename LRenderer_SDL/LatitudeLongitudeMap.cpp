#include "LatitudeLongitudeMap.h"
#include <cmath>
#include <algorithm>

// deepseek 生成
Eigen::Vector4f LatitudeLongitudeMap::Sample(const Eigen::Vector3f &direction, SampleType sampleType) const  
{  
   Eigen::Vector2f uv;  
   DetermineUV(direction.normalized(), uv);  
   return SampleData(uv, static_cast<SampleType>(sampleType));
}

// deepseek 生成
Eigen::Vector3f LatitudeLongitudeMap::GetDirection(const Eigen::Vector2f &uv) const
{
    const float phi = 2 * M_PI * uv.x() + M_PI / 2;
    const float theta = M_PI * (1 - uv.y());

    const float sinTheta = std::sin(theta);
    return Eigen::Vector3f(
        sinTheta * std::cos(phi),
        std::cos(theta),
        sinTheta * std::sin(phi)
    ).normalized();
}

// deepseek 生成
void LatitudeLongitudeMap::DetermineUV(const Eigen::Vector3f &dir, Eigen::Vector2f &uv) const
{
    const Eigen::Vector3f normalized = dir.normalized();

    // 计算经度（0~1）
    const float phi = std::atan2(normalized.z(), normalized.x());
    float x = (phi - M_PI / 2) / (2 * M_PI); 
    x = std::fmod(x + 1.0f, 1.0f);
    if (x > 0.5f)
    {
        x -= 0.5f;
        x *= 1.001f;
        x += 0.50f;
    }
    if (x > 0.96f)
    {
        x -= 0.96;
        x /= 1.01f;
        x += 0.96f;
    }

    uv.x() = x;

    // 计算纬度（0~1）
    float theta = std::acos(normalized.y());
    float y = theta / M_PI;
    if (y > 0.5f)
    {
        y -= 0.5f;
        y /= 1.001f;
        y += 0.50f;
    }
    uv.y() = 1.0f - y;

    // 限制在[0,1]范围内
    uv.x() = std::fmod(uv.x() + 1.0f, 1.0f);
    uv.y() = std::clamp(uv.y(), 0.0f, 1.0f);
}

// deepseek 生成
Eigen::Vector4f LatitudeLongitudeMap::SampleData(const Eigen::Vector2f &uv, SampleType sampleType) const
{
    const auto &targetMap = data;
    const float u = std::clamp(uv.x(), 0.0f, 1.0f);
    const float v = std::clamp(uv.y(), 0.0f, 1.0f);

    if (sampleType == SampleType::Direct)
    {
        int x = static_cast<int>(u * width);
        int y = static_cast<int>(v * height);

        x = std::clamp(x, 0, width - 1);
        y = std::clamp(y, 0, height - 1);

        return data[y * width + x];
    }

    // Bilinear sampling
    const float x = u * width;
    const float y = v * height;

    const int x0 = std::clamp(static_cast<int>(x), 0, width - 1);
    const int y0 = std::clamp(static_cast<int>(y), 0, height - 1);
    const int x1 = std::clamp(x0 + 1, 0, width - 1);
    const int y1 = std::clamp(y0 + 1, 0, height - 1);

    const float fracX = x - x0;
    const float fracY = y - y0;

    const Eigen::Vector4f c00 = data[y0 * width + x0];
    const Eigen::Vector4f c01 = data[y0 * width + x1];
    const Eigen::Vector4f c10 = data[y1 * width + x0];
    const Eigen::Vector4f c11 = data[y1 * width + x1];

    return c00 * (1 - fracX) * (1 - fracY) +
        c01 * fracX * (1 - fracY) +
        c10 * (1 - fracX) * fracY +
        c11 * fracX * fracY;
}