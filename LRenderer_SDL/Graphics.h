#pragma once
#include "Mesh.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "PerspectiveCorrectInterpolation.h"

class Graphics
{
public:
	static void SetFramebuffer(Framebuffer* framebuffer);
	static void SetZBuffer(Buffer<float>* zBuffer);

	static void DrawMesh(const Mesh* mesh, const Eigen::Matrix4f& modelMatrix, Shader* shader);

private:
	static Framebuffer* framebuffer;
	static Buffer<float>* zBuffer;
};

