#pragma once
#include <vector>
#include <cstdint>

#include "Eigen/Dense"
#include "Color.h"
#include "GraphicsSettings.h"

class Shader;

struct appdata
{
    Eigen::Vector4f vertex;
    Eigen::Vector4f normal;
    Eigen::Vector2f uv0;
    Eigen::Vector2f uv1;
    Eigen::Vector2f uv2;
    Eigen::Vector2f uv3;
};

struct v2f
{
    Eigen::Vector4f vertex;
    Eigen::Vector4f texcoords[V2F_TEX_COUNT];
};

struct SubpixelData
{
    // 定义采样点
    Eigen::Vector2f screenPosition;

    Eigen::Vector4f color;
    float z;

    std::intptr_t tempData;

    Shader *shader; // 不为空表示片元有带渲染颜色
    v2f builtinV2f;
    v2f v2f;
    
    SubpixelData() 
    {
        shader = nullptr;
    };
    SubpixelData(Eigen::Vector2f screenPosition);
    void Reset(Eigen::Vector2f screenPosition);
};

struct PixelData
{
    std::vector<SubpixelData> subpixels;
};

struct TAASubpixelData
{
    Eigen::Vector4f historyColor;
    uint8_t sampleCount;
};

struct TAAData
{
    // TODO: 锚点颜色
    Eigen::Vector4f anchorColor;

    TAASubpixelData subpixels[MSAA_TYPE];
};
