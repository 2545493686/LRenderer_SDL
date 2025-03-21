#pragma once
#include "Eigen/Dense"

class Cubemap
{
public:
    enum class SampleType
    {
        Direct,
        Bilinear
    };

    enum Face 
    {
        PosX, 
        NegX, 
        PosY, 
        NegY, 
        PosZ, 
        NegZ 
    };

    explicit Cubemap(int faceSize) : size(faceSize) 
    {
        for (int i = 0; i < 6; ++i) {
            data[i] = new Eigen::Vector4f[size * size];
        }
    }

    ~Cubemap() 
    {
        for (int i = 0; i < 6; ++i) {
            delete[] data[i];
            data[i] = nullptr;
        }
    }

    std::vector<Cubemap *> mipmaps = std::vector<Cubemap *>();
    std::vector<Cubemap *> radianceMaps = std::vector<Cubemap *>();
    Eigen::Vector4f* data[6] = { nullptr };
    int size = 0;

    Eigen::Vector4f SampleByRoughness(const Eigen::Vector3f &direction, float roughness) const;

    Eigen::Vector4f Sample(const Eigen::Vector3f& direction, SampleType sampleType = SampleType::Direct, int mipmapsLayer = 0) const;
    Eigen::Vector3f GetDirection(Face face, const Eigen::Vector2f& uv) const;
    Eigen::Vector3f SetRadianceMaps(std::vector<Cubemap *> radianceMaps);
    Eigen::Vector3f SetMipmaps(std::vector<Cubemap *> mipmaps);

    Eigen::Vector4f SampleFace(Face face, const Eigen::Vector2f& uv, SampleType sampleType = SampleType::Direct, int mipmapsLayer = 0) const;
    Eigen::Vector4f SampleFace(Face face, int x, int y, int mipmapsLayer = 0) const;

    EIGEN_ALWAYS_INLINE void PutPixel(Face face, int x, int y, const Eigen::Vector4f &color)
    {
        data[static_cast<int>(face)][y * size + x] = color;
    }

    // deepseek 生成
    EIGEN_ALWAYS_INLINE void PutPixel(Face face, const Eigen::Vector2f& uv, const Eigen::Vector4f& color)
    {
        // Clamp UV坐标到[0, 1]范围
        float u = std::clamp(uv.x(), 0.0f, 1.0f);
        float v = std::clamp(uv.y(), 0.0f, 1.0f);

        // 计算对应的像素坐标
        float x = u * size;
        float y = v * size;

        // 转换为整数坐标并进行Clamp处理
        int x0 = static_cast<int>(x);
        int y0 = static_cast<int>(y);
        x0 = std::max(0, std::min(size - 1, x0));
        y0 = std::max(0, std::min(size - 1, y0));

        // 写入颜色到对应的面数据
        data[static_cast<int>(face)][y0 * size + x0] = color;
    }

    EIGEN_ALWAYS_INLINE int GetSize() const 
    { 
        return size; 
    }

private:
    void DetermineFaceAndUV(const Eigen::Vector3f& dir, Face& face, Eigen::Vector2f& uv) const;
};

