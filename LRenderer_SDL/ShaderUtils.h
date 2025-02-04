#pragma once

#include "Eigen/Dense"
#include "EnvVariable.h"

// 使用前先调用 SetContext 设置上下文
class ShaderUtils
{
	ShaderUtils() = delete;

public:
	static EIGEN_ALWAYS_INLINE 
	Eigen::Vector4f ToClipPos(const EnvVariable* context, Eigen::Vector4f vertex)
	{
		return context->modelToClipMatrix * vertex;
	}
};

