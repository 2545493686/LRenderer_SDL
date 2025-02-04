#include "MeshConverter.h"

// TODO: 非三角化模型导入
Mesh* MeshConverter::Covert(const aiMesh* aiMesh)
{
	Mesh *mesh = new Mesh();

	mesh->verticesCount = aiMesh->mNumVertices;
	mesh->vertices = new Eigen::Vector3f[mesh->verticesCount];

	// 获取顶点
	for (size_t i = 0; i < aiMesh->mNumVertices; i++)
	{
		mesh->vertices[i] = Eigen::Vector3f(
			aiMesh->mVertices[i].x, 
			aiMesh->mVertices[i].y, 
			aiMesh->mVertices[i].z);
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

	return mesh;
}
