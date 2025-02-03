#pragma once

#include "Eigen/Dense"

class Transform
{
public:
	Eigen::Vector3f position;
	Eigen::Quaternionf rotation;
	Eigen::Vector3f scale;

	EIGEN_ALWAYS_INLINE Eigen::Matrix4f GetModelMatrix() const;
};