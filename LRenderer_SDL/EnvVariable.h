#pragma once

#include "Eigen/Dense"

class EnvVariable
{
public:
	Eigen::Matrix4f modelMatrix;
	Eigen::Matrix4f viewMatrix;
	Eigen::Matrix4f frustumMatrix;

	Eigen::Matrix4f modelToClipMatrix;
};