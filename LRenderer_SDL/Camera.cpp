#include "Camera.h"

Camera* Camera::main = nullptr;

Camera::Camera(Transform* transform)
{
	if (Camera::main == nullptr)
	{
		Camera::main = this;
	}

	Camera::transform = transform;
}
