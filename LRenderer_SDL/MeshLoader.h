#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>  // For file sink

#include "Mesh.h"

class MeshLoader
{
public:
    MeshLoader() = delete;

	static Mesh* Load(const char* filename);
    

private:
	static Mesh* LoadByAIMesh(const char* filename);
	static Mesh* CovertAIMesh(const aiMesh* aiMesh);
};