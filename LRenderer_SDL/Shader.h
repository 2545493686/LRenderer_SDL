#pragma once
#include "Eigen/Dense"
#include "EnvContext.h"
#include "ShaderUtils.h"
#include "GraphicsSettings.h"
#include "GraphicsType.h"

// 注意：任何着色器都不应该保留状态
class Shader
{
public:
	int usedTexCount = V2F_TEX_COUNT;

	// 绘制前调用
	virtual void DrawInit(EnvContext *context);

	//必须完成：顶点 ToClipPos
	virtual v2f vertex(const appdata& v) = 0;

	virtual Eigen::Vector4f fragment(const v2f& i) = 0;

protected: 
	EnvContext *context;
};

