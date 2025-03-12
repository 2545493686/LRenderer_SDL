#pragma once
#include "Eigen/Dense"

#include "Camera.h"

class GraphicsUtils
{
	GraphicsUtils() = delete;

public:
	EIGEN_ALWAYS_INLINE static Eigen::Vector4f 
	InterpolationVector(Eigen::Vector3f barycentric, Eigen::Vector3f verticesZ, float z, Eigen::Matrix<float, 4, 3> verticesData, Camera::Type cameraType)
	{
		if (cameraType == Camera::Type::Perspective)
		{
			// // It = (a * Ia / Za + b * Ib / Zb + c * Ic / Zc) * zt
			Eigen::Vector3f zInverse = verticesZ.cwiseInverse();
			Eigen::Vector3f k = barycentric.array() * zInverse.array();
			return (verticesData * k) * z;
		}
		if (cameraType == Camera::Type::Orthographic)
		{
			return (verticesData * barycentric);
		}
		
		assert(false);
	}
};

