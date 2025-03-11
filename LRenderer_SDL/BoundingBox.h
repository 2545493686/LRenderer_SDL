#pragma once
#include "Eigen/Dense"

struct SphereBoundingBox
{
    Eigen::Vector3f center;
    float radius;
};
