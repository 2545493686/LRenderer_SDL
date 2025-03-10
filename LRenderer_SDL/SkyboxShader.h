#pragma once
#include <iostream>
#include "Shader.h"
#include "Cubemap.h"
#include "MathUtils.h"

class SkyboxShader : public Shader
{
public:
    Cubemap *tex1;

	// Inherited via Shader
	v2f vertex(const appdata& v) override;
	Eigen::Vector4f fragment(const v2f& i) override;
};

