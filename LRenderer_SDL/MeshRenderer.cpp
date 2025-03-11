#include "MeshRenderer.h"

MeshRenderer::MeshRenderer(Transform* transform)
{
    this->transform = transform;
}

SphereBoundingBox MeshRenderer::GetSphereBoundingBox()
{
    auto sbb = mesh->sphereBoundingBox;

    Eigen::Vector4f center;
    center << sbb.center, 1.0;
    center = transform->GetModelMatrix() * center;
    sbb.center = center.head<3>();
    sbb.radius = std::max({
            sbb.radius * transform->scale.x(),
            sbb.radius * transform->scale.y(),
            sbb.radius * transform->scale.z()
        });

    return sbb;
}
