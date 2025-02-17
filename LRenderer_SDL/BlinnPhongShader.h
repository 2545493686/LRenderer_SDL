#pragma once
#include "Shader.h"
#include "Texture.h"
#include "Color.h"

class BlinnPhongShader : public Shader
{
public:
	Texture *tex1;
	Eigen::Vector4f diffuse = Color::MakeVector(Color::White);

	// Inherited via Shader
	v2f vertex(const appdata& v) override;
	Eigen::Vector4f fragment(const v2f& i) override;
};

