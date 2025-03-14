#pragma once

#include "Eigen/Dense"
#include "EnvContext.h"
#include "Graphics.h"

class ShaderUtils
{
	ShaderUtils() = delete;

public:
	static EIGEN_ALWAYS_INLINE
	Eigen::Vector4f ToClipPos(const EnvContext *context, Eigen::Vector4f vertex)
	{
		return context->modelToClipMatrix * vertex;
	}

	static EIGEN_ALWAYS_INLINE 
	Eigen::Vector4f ToViewPos(const EnvContext *context, Eigen::Vector4f vertex)
	{
		return context->modelToViewMatrix * vertex;
	}

	static EIGEN_ALWAYS_INLINE
	Eigen::Vector4f ToWorldPos(const EnvContext *context, Eigen::Vector4f vertex)
	{
		return context->modelMatrix * vertex;
	}

	static EIGEN_ALWAYS_INLINE
    float GetVisibility(const EnvContext *context, int lightIndex)
    {
		return Graphics::GetDirectVisibility(context->screenPosition, lightIndex);
    }
};

