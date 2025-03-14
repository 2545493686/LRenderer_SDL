#pragma once
#include "Eigen/Dense"
#include "Color.h"

enum class LightType
{
	Directional
};

class Light
{
public:
	virtual LightType GetLightType() = 0;
};

class DirectionalLight : public Light
{
public:
	Eigen::Vector4f direction;
	Eigen::Vector4f color;
	float intensity = 1;

	LightType GetLightType() override
	{
		return LightType::Directional;
	}

	DirectionalLight()
	{
		direction << 0, -1, -1, 0;
		color = Color::MakeVector(Color::White);
	}
};

