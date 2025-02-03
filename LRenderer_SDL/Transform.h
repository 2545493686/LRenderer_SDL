#pragma once

#include "Eigen/Dense"

class Transform
{
public:
	Eigen::Vector3f position;
	Eigen::Quaternionf rotation;
	Eigen::Vector3f scale;

	Eigen::Matrix4f GetModelMatrix() const;
};