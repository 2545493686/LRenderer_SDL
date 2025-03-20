#pragma once

#include "Shader.h"
#include "Texture.h"

class Cubemap;

class CookTorranceShader : public Shader
{
public:
	float metallic = 0;
	float smoothness = 0.5f;

	Texture *tex1;
	Eigen::Vector4f diffuse = Color::MakeVector(Color::White);

	Cubemap *irradianceTex;

	// Inherited via Shader
	v2f vertex(const appdata &v) override;
	Eigen::Vector4f fragment(const v2f &i) override;
};

