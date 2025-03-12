#pragma once
#include <vector>

#include "Shader.h"
#include "Eigen/Dense"
#include "Color.h"

struct SubpixelData
{
    // 定义采样点
    Eigen::Vector2f screenPosition;

    Eigen::Vector4f color;
    float z;

    int shadowSubIndex;

    Shader* shader = nullptr; // 不为空表示片元有带渲染颜色
    v2f builtinV2f;
    v2f v2f;
    
    SubpixelData() {};
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
