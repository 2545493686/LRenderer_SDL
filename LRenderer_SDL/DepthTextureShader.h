#pragma once
#include <iostream>
#include "Shader.h"

class DepthTextureShader : public Shader
{
public:
	//必须完成：顶点 ToClipPos
	virtual v2f vertex(const appdata& v) override;
	virtual Eigen::Vector4f fragment(const v2f& i) override;
};

