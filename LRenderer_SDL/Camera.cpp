#include "Camera.h"

Camera::Camera(Transform* transform)
{
	Camera::transform = transform;
}

EIGEN_ALWAYS_INLINE Eigen::Matrix4f Camera::GetViewMatrix() const
{
	Eigen::Affine3f affine = Eigen::Affine3f::Identity();
	affine.translate(-transform->position);
	affine.rotate(transform->rotation.inverse());
	return affine.matrix();
}
