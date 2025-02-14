#pragma once
#include "Mesh.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "PerspectiveCorrectInterpolation.h"
#include "Random.h"
#include "spdlog/spdlog.h"
#include "GraphicsSettings.h"

class Graphics
{
public:
	// 输出
	static void SetFramebuffer(Framebuffer *framebuffer);
	static void DrawMesh(const Mesh* mesh, const Eigen::Matrix4f& modelMatrix, Shader* shader);

private:
	static Framebuffer* framebuffer;

	static Eigen::Vector2f GetSubpixelPointBias(int x, int y, int i);

	static const Eigen::Vector2f subpixelBiasX4[4];
};

EIGEN_ALWAYS_INLINE Eigen::Vector2f Graphics::GetSubpixelPointBias(int x, int y, int subpixelIndex)
{
#if MSAA_TYPE == MSAA_X4
	return subpixelBiasX4[subpixelIndex];
#endif // MSAA_TYPE == MSAA_X4

	throw std::exception("msaaCount is unknown type.");
}


