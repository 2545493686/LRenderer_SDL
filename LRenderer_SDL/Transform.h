#pragma once

#define _USE_MATH_DEFINES

#include <cmath>
#include "Eigen/Dense"
#include "Component.h"

class Transform : public Component
{
public:
	Eigen::Vector3f position = Eigen::Vector3f(0, 0, 0);
	Eigen::Quaternionf rotation = Eigen::Quaternionf::Identity();
	Eigen::Vector3f scale = Eigen::Vector3f(1, 1, 1);

	void Rotate(float x, float y, float z);

	EIGEN_ALWAYS_INLINE Eigen::Matrix4f GetModelMatrix() const
	{
		Eigen::Affine3f affine = Eigen::Affine3f::Identity();
		affine.translate(position);
		affine.rotate(rotation);
		affine.scale(scale);

		return affine.matrix();
	}
};