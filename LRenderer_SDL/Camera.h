#pragma once
#define _USE_MATH_DEFINES

#include "Component.h"
#include "Transform.h"
#include "Eigen/Dense"
#include "cmath"

class Camera : Component
{
public:
	float fov = 45;
	float aspect = 1;
	float zNear = 0.1f;
	float zFar = 50;

	Camera(Transform* transform);
	Eigen::Matrix4f GetViewMatrix() const;
	Eigen::Matrix4f GetProjectionMatrix() const;

private:
	Transform *transform;
};

