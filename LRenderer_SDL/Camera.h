#pragma once
#include "Component.h"
#include "Transform.h"
#include "Eigen/Dense"

class Camera : Component
{
public:
	Camera(Transform* transform);
	Eigen::Matrix4f GetViewMatrix() const;

private:
	Transform *transform;
};

