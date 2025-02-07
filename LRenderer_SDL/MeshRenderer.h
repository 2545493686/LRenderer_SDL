#pragma once
#include "Component.h"
#include "Mesh.h"
#include "Shader.h"

// TODO: 要求Transform组件必须存在
class MeshRenderer : public Component
{
public:
	Mesh *mesh;
	Shader *shader;
};

