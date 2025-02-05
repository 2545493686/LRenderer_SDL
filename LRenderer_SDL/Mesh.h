#pragma once
#include "Eigen/Dense"

class Mesh
{
public:
	Eigen::Vector3f *vertices = nullptr;
	int verticesCount;

	int *edges = nullptr;
	int edgesCount;

	Eigen::Vector2f* uv0 = nullptr;
	Eigen::Vector2f* uv1 = nullptr;
	Eigen::Vector2f* uv2 = nullptr;
	Eigen::Vector2f* uv3 = nullptr;

	Eigen::Vector2f** uvs[4] = {
		&uv0, &uv1, &uv2, &uv3,
	};
};

