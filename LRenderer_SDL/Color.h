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
	static const uint32_t LightYellow = 0xFFFFFF99;

	EIGEN_ALWAYS_INLINE static Eigen::Vector4f MakeVector(uint32_t color)
	{
        float a = static_cast<float>((color >> 24) & 0xFF) / 255.0f;
		float r = static_cast<float>((color >> 16) & 0xFF) / 255.0f;
        float g = static_cast<float>((color >> 8) & 0xFF) / 255.0f;
        float b = static_cast<float>((color >> 0) & 0xFF) / 255.0f;
        return Eigen::Vector4f(r, g, b, a);
	}

	EIGEN_ALWAYS_INLINE static Eigen::Vector4f MakeVector(float r, float g, float b, float a)
	{
        return Eigen::Vector4f(r, g, b, a);
	}


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
		return (MathUtils::Clamp<int>(static_cast<uint32_t>(a * 255.0f), 0, 255) << 24)
		| (MathUtils::Clamp<int>(static_cast<uint32_t>(r * 255.0f), 0, 255) << 16)
		| (MathUtils::Clamp<int>(static_cast<uint32_t>(g * 255.0f), 0, 255) << 8)
		| (MathUtils::Clamp<int>(static_cast<uint32_t>(b * 255.0f), 0, 255));
	}
};

