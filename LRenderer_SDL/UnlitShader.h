#pragma once
#include "Shader.h"
#include "ShaderUtils.h"
#include "Texture.h"

class UnlitShader : public Shader
{
public:
	Texture *tex1;

	v2f vertex(const appdata &v) override;
	Eigen::Vector4f fragment(const v2f &i) override;
};

