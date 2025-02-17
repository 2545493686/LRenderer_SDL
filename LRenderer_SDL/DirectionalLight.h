#pragma once

#include <Eigen/Dense>

#include "Color.h"
#include "Light.h"

class DirectionalLight : public Light
{
public:
	Eigen::Vector4f direction;
	Eigen::Vector4f color;
	float intensity = 1;

	DirectionalLight()
	{
		direction << 0, -1, -1, 0;
		color = Color::MakeVector(Color::White);
	}
};

