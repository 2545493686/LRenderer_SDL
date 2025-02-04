#pragma once
#include "Eigen/Dense"
#include "EnvVariable.h"

struct appdata
{
	Eigen::Vector4f vertex;
};

struct v2f
{
	Eigen::Vector4f vertex;
};

class Shader
{
public:
	// 绘制前调用
	virtual void DrawInit(EnvVariable *context);

	//必须完成：顶点 ToClipPos
	virtual v2f* vertex(const appdata* v) = 0;

	virtual Eigen::Vector4f fragment(const v2f* i) = 0;

protected: 
	EnvVariable *context;
};

