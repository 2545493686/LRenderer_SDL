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

// 从 games101 作业抄的; LANQ 25.2.3
Eigen::Matrix4f Camera::GetProjectionMatrix() const
{
    Eigen::Matrix4f p = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f mortho = Eigen::Matrix4f::Identity();

    float nSize = tan(fov / 2 * M_PI / 180) * zNear;

    p <<    zNear,  0,      0,              0,
            0,      zNear,  0,              0,
            0,      0,      zNear + zFar,   -zFar * zNear,
            0,      0,      1,              0;

    mortho <<   1 / (aspect * nSize),   0,          0,                  0,
                0,                      1 / nSize,  0,                  0,
                0,                      0,          2 / (zFar - zNear), -(zFar + zNear) / 2,
                0,                      0,          0,                  1;

    Eigen::Matrix4f projection = mortho * p;

    return projection;
}
