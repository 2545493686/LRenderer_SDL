#pragma once
#include "Shader.h";
#include "Cubemap.h"

class CubemapShader : public Shader
{
public:
	Cubemap *cubemap;

	// Inherited via Shader
	v2f vertex(const appdata& v) override;
	Eigen::Vector4f fragment(const v2f& i) override;
};

