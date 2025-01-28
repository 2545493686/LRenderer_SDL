#pragma once
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class MeshConverter
{
public:
	static Mesh& Covert(const aiMesh *aiMesh);
};

