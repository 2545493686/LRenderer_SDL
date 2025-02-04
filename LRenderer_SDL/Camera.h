#pragma once
#define _USE_MATH_DEFINES

#include "Component.h"
#include "Transform.h"
#include "Eigen/Dense"
#include "cmath"

class Camera : public Component
{
public:
    static Camera* main;

	float fov = 45;
	float aspect = 1;
	float zNear = 0.1f;
	float zFar = 50;

	Camera(Transform* transform);
	
	EIGEN_ALWAYS_INLINE Eigen::Matrix4f GetViewMatrix() const
	{
		Eigen::Affine3f affine = Eigen::Affine3f::Identity();
		affine.translate(-transform->position);
		affine.rotate(transform->rotation.inverse());
		return affine.matrix();
	}

	EIGEN_ALWAYS_INLINE Eigen::Matrix4f GetFrustumMatrix() const
	{
		Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

		float nSize = 1 / tan(fov / 2);

		projection << nSize / aspect, 0, 0, 0,
			0, nSize, 0, 0,
			0, 0, (zFar + zNear) / (zFar - zNear), -2 * zFar * zNear / (zFar - zNear),
			0, 0, -1, 0;

		return projection;
	}

private:
	Transform *transform;
};

