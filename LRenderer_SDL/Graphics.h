#pragma once
#include "Mesh.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "PerspectiveCorrectInterpolation.h"

class Graphics
{
public:
	struct FragmentData
	{
		uint16_t sampleCount;
		Eigen::Vector4f anchor;

		Eigen::Vector4f subColors[4];
		uint8_t subSampleCount[4];
	};

	static void SetFramebuffer(Framebuffer* framebuffer);
	static void SetZBuffer(Buffer<float>* zBuffer);
	static void SetVectorMotionBuffer(Buffer<Eigen::Vector3f>* motionVectorBuffer);

	static void DrawMesh(const Mesh* mesh, const Eigen::Matrix4f& modelMatrix, Shader* shader);

private:
	static Framebuffer* framebuffer;
	static Buffer<float>* zBuffer;
	static Buffer<Eigen::Vector3f>* motionVectorBuffer;
};

