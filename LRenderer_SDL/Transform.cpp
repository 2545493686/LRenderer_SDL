#include "Transform.h"

EIGEN_ALWAYS_INLINE Eigen::Matrix4f Transform::GetModelMatrix() const
{
    Eigen::Affine3f affine = Eigen::Affine3f::Identity();
	affine.translate(position);
	affine.rotate(rotation);
	affine.scale(scale);

    return affine.matrix();
}
