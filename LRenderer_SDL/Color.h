#pragma once
#include <cstdint>
#include <algorithm>

#include "Eigen/Dense"
#include "MathUtils.h"

class Color
{
public:
	static const uint32_t Red = 0xFFFF0000;
	static const uint32_t Green = 0xFF00FF00;
	static const uint32_t Blue = 0xFF0000FF;
	static const uint32_t White = 0xFFFFFFFF;
	static const uint32_t Black = 0xFF000000;
	static const uint32_t Yellow = 0xFFFFFF00;
	static const uint32_t Cyan = 0xFF00FFFF;
	static const uint32_t Magenta = 0xFFFF00FF;
	static const uint32_t Gray = 0xFF808080;
	static const uint32_t LightGray = 0xFFD3D3D3;

	EIGEN_ALWAYS_INLINE static uint32_t Make(float color)
	{
		return Make(color, color, color, 1.0f);
	}

	EIGEN_ALWAYS_INLINE static uint32_t Make(Eigen::Vector4f color)
	{
		return Make(color.x(), color.y(), color.z(), color.w());
	}

	EIGEN_ALWAYS_INLINE static uint32_t Make(float r, float g, float b)
	{
		return Make(r, g, b, 1.0f);
	}

	EIGEN_ALWAYS_INLINE static uint32_t Make(float r, float g, float b, float a)
	{
		return (static_cast<uint32_t>(MathUtils::Clamp<float>(a * 255.0f, 0.0f, 255.0f)) << 24)
			| (static_cast<uint32_t>(MathUtils::Clamp<float>(r * 255.0f, 0.0f, 255.0f)) << 16)
			| (static_cast<uint32_t>(MathUtils::Clamp<float>(g * 255.0f, 0.0f, 255.0f)) << 8)
			| (static_cast<uint32_t>(MathUtils::Clamp<float>(b * 255.0f, 0.0f, 255.0f)));
	}
};

