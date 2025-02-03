#include "Transform.h"

EIGEN_ALWAYS_INLINE Eigen::Matrix4f Transform::GetModelMatrix() const
{
    Eigen::Affine3f transform = Eigen::Affine3f::Identity();
	transform.translate(position);
	transform.rotate(rotation);
	transform.scale(scale);

    return transform.matrix();
}
