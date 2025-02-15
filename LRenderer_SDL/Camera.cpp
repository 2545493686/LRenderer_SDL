#include "Camera.h"

Camera* Camera::main = nullptr;

Camera::Camera(Transform* transform)
{
	if (this->main == nullptr)
	{
		this->main = this;
	}

	this->transform = transform;
}
