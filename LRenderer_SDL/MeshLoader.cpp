#include "MeshLoader.h"

Mesh* MeshLoader::Load(const char* filename)
{
    return LoadByAIMesh(filename);
}

Mesh* MeshLoader::LoadByAIMesh(const char* filename)
{
	Assimp::Importer importer;
	const aiScene* aiscene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals);
	if (!aiscene)
	{
		spdlog::error(importer.GetErrorString());
		abort();
	}
	const aiMesh* cubeAiMesh = aiscene->mMeshes[0];
	return CovertAIMesh(cubeAiMesh);
}

Mesh* MeshLoader::CovertAIMesh(const aiMesh* aiMesh)
{
	Mesh* mesh = new Mesh();

	mesh->verticesCount = aiMesh->mNumVertices;
	mesh->vertices = new Eigen::Vector3f[mesh->verticesCount];

	// 获取顶点
	for (size_t i = 0; i < aiMesh->mNumVertices; i++)
	{
		mesh->vertices[i] <<  
			aiMesh->mVertices[i].x,
			aiMesh->mVertices[i].y,
			aiMesh->mVertices[i].z;
	}

	// 获取边
	mesh->edgesCount = aiMesh->mNumFaces * 3;
	mesh->edges = new int[mesh->edgesCount];
	for (size_t i = 0; i < aiMesh->mNumFaces; i++)
	{
		mesh->edges[i * 3] = aiMesh->mFaces[i].mIndices[0];
		mesh->edges[i * 3 + 1] = aiMesh->mFaces[i].mIndices[1];
		mesh->edges[i * 3 + 2] = aiMesh->mFaces[i].mIndices[2];
	}

	// 获取uv
	for (size_t i = 0; i < 4; i++)
	{
		*(mesh->uvs[i]) = new Eigen::Vector2f[mesh->verticesCount];

		if (aiMesh->HasTextureCoords(i))
		{
			for (size_t j = 0; j < mesh->verticesCount; j++)
			{
				(*(mesh->uvs[i]))[j] << 
					aiMesh->mTextureCoords[i][j].x,
					aiMesh->mTextureCoords[i][j].y;
			}
		}
	}

	// 获取法线
	mesh->normals = new Eigen::Vector3f[mesh->verticesCount];
	if (aiMesh->mNormals)
	{
		for (size_t i = 0; i < aiMesh->mNumVertices; i++)
		{
			mesh->normals[i] << 
				aiMesh->mNormals[i].x, 
				aiMesh->mNormals[i].y,
				aiMesh->mNormals[i].z;
		}
	}
	else
	{
		// TODO: 通过面计算法线，必要时需要新增顶点
	}


	return mesh;
}
