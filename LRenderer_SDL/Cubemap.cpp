#include "Cubemap.h"
#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>  // For file sink

Eigen::Vector4f Cubemap::SampleByRoughness(const Eigen::Vector3f &direction, float roughness) const
{
    const Eigen::Vector3f dir = direction.normalized();
    Face face;
    Eigen::Vector2f uv;
    DetermineFaceAndUV(dir, face, uv);

	auto mipmapsLayer = roughness * roughness * (radianceMaps.size() - 2);

    Eigen::Vector4f colors[2];
    for (size_t i = 0; i < 2; i++)
    {
        int layer = static_cast<int>(mipmapsLayer) + i;
        auto size = radianceMaps[layer]->size;

        float x = uv.x() * size;
        float y = uv.y() * size;

        int x0 = static_cast<int>(x);
        int y0 = static_cast<int>(y);

        x0 = std::clamp(x0, 0, size - 1);
        y0 = std::clamp(y0, 0, size - 1);

        int x1 = std::clamp(x0 + 1, 0, size - 1);
        int y1 = std::clamp(y0 + 1, 0, size - 1);
        
        float u = x - x0;
        float v = y - y0;
        
        Eigen::Vector4f c00 = radianceMaps[layer]->data[static_cast<int>(face)][y0 * size + x0];
        Eigen::Vector4f c01 = radianceMaps[layer]->data[static_cast<int>(face)][y0 * size + x1];
        Eigen::Vector4f c10 = radianceMaps[layer]->data[static_cast<int>(face)][y1 * size + x0];
        Eigen::Vector4f c11 = radianceMaps[layer]->data[static_cast<int>(face)][y1 * size + x1];

        colors[i] = c00 * (1 - u) * (1 - v) + c01 * u * (1 - v) + c10 * (1 - u) * v + c11 * u * v;
    }

	return colors[0] * (1 - mipmapsLayer + static_cast<int>(mipmapsLayer)) 
        + colors[1] * (mipmapsLayer - static_cast<int>(mipmapsLayer));
}

Eigen::Vector4f Cubemap::Sample(const Eigen::Vector3f& direction, SampleType sampleType, int mipmapsLayer) const
{
    const Eigen::Vector3f dir = direction.normalized();
    Face face;
    Eigen::Vector2f uv;
    DetermineFaceAndUV(dir, face, uv);
    return SampleFace(face, uv, sampleType, mipmapsLayer);
}

// deepseek 生成
// 输出已归一化
Eigen::Vector3f Cubemap::GetDirection(Face face, const Eigen::Vector2f &uv) const
{
    // 原始 DetermineFaceAndUV 的 UV 计算规则：
    // - X±面：存在 UV 翻转逻辑
    // - Y±面：V坐标方向特殊处理
    // - Z±面：U坐标有反向操作

    const float u = std::clamp(uv.x(), 0.0f, 1.0f);
    const float v = 1.0f - std::clamp(uv.y(), 0.0f, 1.0f);

    switch (face)
    {
        // 每个面的计算都严格逆向推导自 DetermineFaceAndUV
    case Face::NegX: // X+方向（原逻辑中 dir.x() > 0）
    {
        // 原式：uv.x() = (dir.z/(dir.x) + 1)/2 → dir.z = (2u -1)*dir.x
        //        uv.y() = 1 - (dir.y/dir.x +1)/2 → dir.y = (1 - 2*(1-v) -1)*dir.x = (2v -1)*dir.x
        const float dir_x = 1.0f;
        const float dir_y = 2 * v - 1.0f;  // 修复Y分量上下颠倒
        const float dir_z = 2 * u - 1.0f;  // 修复X+左右颠倒
        return Eigen::Vector3f(dir_x, dir_y, dir_z).normalized();
    }

    case Face::PosX: // X-方向（dir.x() < 0）
    {
        // 原式：uv.x() = 1 - [(dir.z/(-dir.x) +1)/2] → dir.z = (1 - 2u)*(-dir.x)
        //        uv.y() = 1 - (dir.y/(-dir.x) +1)/2 → dir.y = (2v -1)*(-dir.x)
        const float dir_x = -1.0f;
        const float dir_y = (2 * v - 1.0f); // 修复Y分量上下颠倒
        const float dir_z = (1.0f - 2 * u) * (-dir_x); // 修复X-左右颠倒
        return Eigen::Vector3f(dir_x, dir_y, dir_z).normalized();
    }

    case Face::PosY: // Y+方向（dir.y() > 0）
    {
        // 原式：uv.x() = 1 - (dir.x/dir.y +1)/2 → dir.x = (1 - 2u)*dir.y
        //        uv.y() = 1 - (-dir.z/dir.y +1)/2 → dir.z = (2v -1)*(-dir.y)
        const float dir_y = 1.0f;
        const float dir_x = (1.0f - 2 * u) * dir_y;  // 修复X分量左右颠倒
        const float dir_z = (2 * v - 1.0f) * (-dir_y);
        return Eigen::Vector3f(dir_x, dir_y, dir_z).normalized();
    }

    case Face::NegY: // Y-方向（dir.y() < 0）
    {
        // 原式：uv.y() = 1 - (dir.z/(-dir.y) +1)/2 → dir.z = (2v -1)*(-dir.y)
        //        uv.x() = 1 - (dir.x/(-dir.y) +1)/2 → dir.x = (2u -1)*(-dir.y)
        const float dir_y = -1.0f;
        const float dir_x = -(2 * u - 1.0f); // 修复X分量左右颠倒
        const float dir_z = (2 * v - 1.0f); // 修复Z分量上下颠倒
        return Eigen::Vector3f(dir_x, dir_y, dir_z).normalized();
    }

    case Face::PosZ: // Z+方向（dir.z() > 0）
    {
        // 原式：uv.x() = (-dir.x/dir.z +1)/2 → dir.x = (1 - 2u)*dir.z
        //        uv.y() = 1 - (dir.y/dir.z +1)/2 → dir.y = (2v -1)*dir.z
        const float dir_z = 1.0f;
        const float dir_x = (1.0f - 2 * u) * dir_z;  // 修复X分量左右颠倒
        const float dir_y = (2 * v - 1.0f) * dir_z;  // 修复Y分量上下颠倒
        return Eigen::Vector3f(dir_x, dir_y, dir_z).normalized();
    }

    case Face::NegZ: // Z-方向（dir.z() < 0）
    {
        // 原式：uv.x() = 1 - [(-dir.x/(-dir.z) +1)/2] → dir.x = (2u -1)*(-dir.z)
        //        uv.y() = 1 - (dir.y/(-dir.z) +1)/2 → dir.y = (2v -1)*(-dir.z)
        const float dir_z = -1.0f;
        const float dir_x = (2 * u - 1.0f) * (-dir_z); // 修复X分量左右颠倒
        const float dir_y = (2 * v - 1.0f) * (-dir_z); // 修复Y分量上下颠倒
        return Eigen::Vector3f(dir_x, dir_y, dir_z).normalized();
    }

    default:
        throw std::runtime_error("Invalid cubemap face");
    }
}

void Cubemap::SetRadianceMaps(std::vector<Cubemap *> radianceMaps)
{
    this->radianceMaps = radianceMaps;
}

void Cubemap::SetMipmaps(std::vector<Cubemap *> mipmaps)
{
    int s = this->GetSize();
    int i = 0;
    
    while (s)
    {
        i++;
        s /= 2;
    }

    if (mipmaps.size() != i)
    {
        spdlog::error("Invalid mipmap count.");
        return;
    }

    this->mipmaps = mipmaps;
}

// deepseek 生成，已修改
void Cubemap::DetermineFaceAndUV(const Eigen::Vector3f& dir, Face& face, Eigen::Vector2f& uv) const
{
    const Eigen::Vector3f absDir = dir.cwiseAbs();
    float maxAxis = std::max({ absDir.x(), absDir.y(), absDir.z() });

    if (maxAxis == absDir.x()) {
        face = (dir.x() > 0) ? Face::NegX : Face::PosX;
        const float sign = (dir.x() > 0) ? 1.0f : -1.0f;
        uv.y() = 1 - (dir.y() / (sign * dir.x()) + 1.0f) * 0.5f;
        uv.x() = (dir.z() / (sign * dir.x()) + 1.0f) * 0.5f;
        if (sign < 0)
        {
            uv.x() = 1 - uv.x();
        }
    }
    else if (maxAxis == absDir.y()) {
        face = (dir.y() > 0) ? Face::PosY : Face::NegY;
        const float sign = (dir.y() > 0) ? 1.0f : -1.0f;
        uv.y() = 1 - (-dir.z() / (sign * dir.y()) + 1.0f) * 0.5f;
        uv.x() = 1 - (dir.x() / (sign * dir.y()) + 1.0f) * 0.5f;
        if (sign < 0)
        {
            uv.y() = 1 - uv.y();
        }
    }
    else {
        face = (dir.z() > 0) ? Face::PosZ : Face::NegZ;
        const float sign = (dir.z() > 0) ? 1.0f : -1.0f;
        uv.y() = 1 - (dir.y() / (sign * dir.z()) + 1.0f) * 0.5f;
        uv.x() = (-dir.x() / (sign * dir.z()) + 1.0f) * 0.5f;
        if (sign < 0)
        {
            uv.x() = 1 - (-dir.x() / (sign * dir.z()) + 1.0f) * 0.5f;
        }
    }
}

Eigen::Vector4f Cubemap::SampleFace(Face face, int x, int y, int mipmapsLayer) const
{
    return mipmaps[mipmapsLayer]->data[static_cast<int>(face)][y * size + x];
}

Eigen::Vector4f Cubemap::SampleFace(Face face, const Eigen::Vector2f & uv, SampleType sampleType, int mipmapsLayer) const
{
    auto size = mipmaps[mipmapsLayer]->size;
    
    if (sampleType == SampleType::Direct)
    {
        float x = uv.x() * size;
        float y = uv.y() * size;

        int x0 = static_cast<int>(x);
        int y0 = static_cast<int>(y);

        x0 = std::max(0, std::min(size - 1, x0));
        y0 = std::max(0, std::min(size - 1, y0));

        return mipmaps[mipmapsLayer]->data[static_cast<int>(face)][y0 * size + x0];
	}

	if (sampleType == SampleType::Bilinear)
	{
		float x = uv.x() * size;
		float y = uv.y() * size;
		
        int x0 = static_cast<int>(x);
		int y0 = static_cast<int>(y);
		
		x0 = std::clamp(x0, 0, size - 1);
		y0 = std::clamp(y0, 0, size - 1);

        int x1 = std::clamp(x0 + 1, 0, size - 1);
		int y1 = std::clamp(y0 + 1, 0, size - 1);
		float u = x - x0;
		float v = y - y0;
		Eigen::Vector4f c00 = mipmaps[mipmapsLayer]->data[static_cast<int>(face)][y0 * size + x0];
		Eigen::Vector4f c01 = mipmaps[mipmapsLayer]->data[static_cast<int>(face)][y0 * size + x1];
		Eigen::Vector4f c10 = mipmaps[mipmapsLayer]->data[static_cast<int>(face)][y1 * size + x0];
		Eigen::Vector4f c11 = mipmaps[mipmapsLayer]->data[static_cast<int>(face)][y1 * size + x1];
		
        return c00 * (1 - u) * (1 - v) + c01 * u * (1 - v) + c10 * (1 - u) * v + c11 * u * v;
    }

	throw std::runtime_error("Invalid sample type.");
}

