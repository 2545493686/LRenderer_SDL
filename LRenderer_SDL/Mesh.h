#pragma once
#include "Eigen/Dense"

class Mesh
{
public:
	Eigen::Vector3f *vertices;
	int verticesCount;

	int *edges;
	int edgesCount;
};

