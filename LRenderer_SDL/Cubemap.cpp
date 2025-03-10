#include "Cubemap.h"

Eigen::Vector4f Cubemap::Sample(const Eigen::Vector3f& direction) const
{
    const Eigen::Vector3f dir = direction.normalized();
    Face face;
    Eigen::Vector2f uv;
    DetermineFaceAndUV(dir, face, uv);
    return SampleFace(face, uv);
}

void Cubemap::DetermineFaceAndUV(const Eigen::Vector3f& dir, Face& face, Eigen::Vector2f& uv) const
{
    const Eigen::Vector3f absDir = dir.cwiseAbs();
    float maxAxis = std::max({ absDir.x(), absDir.y(), absDir.z() });

    if (maxAxis == absDir.x()) {
        face = (dir.x() > 0) ? Face::NegX : Face::PosX;
        const float sign = (dir.x() > 0) ? 1.0f : -1.0f;
        uv.y() = 1 - (dir.y() / (sign * dir.x()) + 1.0f) * 0.5f;
        uv.x() = (dir.z() / (sign * dir.x()) + 1.0f) * 0.5f;
        if (sign < 0)
        {
            uv.x() = 1 - uv.x();
        }
    }
    else if (maxAxis == absDir.y()) {
        face = (dir.y() > 0) ? Face::PosY : Face::NegY;
        const float sign = (dir.y() > 0) ? 1.0f : -1.0f;
        uv.y() = 1 - (-dir.z() / (sign * dir.y()) + 1.0f) * 0.5f;
        uv.x() = 1 - (dir.x() / (sign * dir.y()) + 1.0f) * 0.5f;
        if (sign < 0)
        {
            uv.y() = 1 - uv.y();
        }
    }
    else {
        face = (dir.z() > 0) ? Face::PosZ : Face::NegZ;
        const float sign = (dir.z() > 0) ? 1.0f : -1.0f;
        uv.y() = 1 - (dir.y() / (sign * dir.z()) + 1.0f) * 0.5f;
        uv.x() = (-dir.x() / (sign * dir.z()) + 1.0f) * 0.5f;
        if (sign < 0)
        {
            uv.x() = 1 - (-dir.x() / (sign * dir.z()) + 1.0f) * 0.5f;
        }
    }
}

Eigen::Vector4f Cubemap::SampleFace(Face face, const Eigen::Vector2f & uv) const 
{
    float x = uv.x() * size;
    float y = uv.y() * size;

    int x0 = static_cast<int>(x);
    int y0 = static_cast<int>(y);

    x0 = std::max(0, std::min(size - 1, x0));
    y0 = std::max(0, std::min(size - 1, y0));

    Eigen::Vector4f p00 = data[static_cast<int>(face)][y0 * size + x0];

    return p00;
}

