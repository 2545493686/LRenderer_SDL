#pragma once
#include "Shader.h"
#include "ShaderUtils.h"

class UnlitShader : public Shader
{
	v2f vertex(const appdata &v) override;
	Eigen::Vector4f fragment(const v2f &i) override;
};

