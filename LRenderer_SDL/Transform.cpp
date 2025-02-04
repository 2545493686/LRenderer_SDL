#include "Transform.h"

void Transform::Rotate(float x, float y, float z)
{
	Eigen::Quaternionf q = Eigen::AngleAxisf(x, Eigen::Vector3f::UnitX()) *
		Eigen::AngleAxisf(y, Eigen::Vector3f::UnitY()) *
		Eigen::AngleAxisf(z, Eigen::Vector3f::UnitZ());
	rotation = q * rotation;
}
