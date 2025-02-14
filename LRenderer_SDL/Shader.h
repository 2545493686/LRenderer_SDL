#pragma once
#include "Eigen/Dense"
#include "EnvVariable.h"
#include "ShaderUtils.h"

struct appdata
{
	Eigen::Vector4f vertex;
	Eigen::Vector2f uv0;
	Eigen::Vector2f uv1;
	Eigen::Vector2f uv2;
	Eigen::Vector2f uv3;
};

struct v2f
{
	Eigen::Vector4f vertex;
	Eigen::Vector4f texcoords[4];
};

// 注意：任何着色器都不应该保留状态
class Shader
{
public:
	// 绘制前调用
	virtual void DrawInit(EnvVariable *context);

	//必须完成：顶点 ToClipPos
	virtual v2f vertex(const appdata& v) = 0;

	virtual Eigen::Vector4f fragment(const v2f& i) = 0;

protected: 
	EnvVariable *context;
};

