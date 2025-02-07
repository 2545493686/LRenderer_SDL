#include "Transform.h"

// TODO: 有严重bug，根本没实现对
void Transform::Rotate(float x, float y, float z)
{
	x *= M_PI / 180.0f;
	y *= M_PI / 180.0f;
	z *= M_PI / 180.0f;

	Eigen::Quaternionf q = 
		Eigen::AngleAxisf(x, Eigen::Vector3f::UnitX()) *
		Eigen::AngleAxisf(y, Eigen::Vector3f::UnitY()) *
		Eigen::AngleAxisf(z, Eigen::Vector3f::UnitZ());
	rotation = q * rotation;
}
