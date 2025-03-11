#pragma once

#include <algorithm>

#include "Component.h"
#include "Mesh.h"
#include "Shader.h"
#include "Transform.h"

// TODO: 要求Transform组件必须存在
class MeshRenderer : public Component
{
public:
	Mesh *mesh;
	Shader *shader;
	Transform *transform;

	MeshRenderer(Transform *transform);

	// 世界空间包围盒
	SphereBoundingBox GetSphereBoundingBox();
};

