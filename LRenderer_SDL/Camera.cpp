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

// 从入门精要抄的 P79 ; LANQ 25.2.4
// 到齐次裁剪空间，未经过透视除法，改变旋性到左手系
Eigen::Matrix4f Camera::GetFrustumMatrix() const
{
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

	float nSize = 1 / tan(fov / 2);

    projection <<	nSize / aspect,	0,          0,									0,
                    0,              nSize,		0,									0,
                    0,              0,          (zFar + zNear) / (zFar - zNear),	- 2 * zFar * zNear / (zFar - zNear),
                    0,              0,          -1,									0;

    return projection;
}
