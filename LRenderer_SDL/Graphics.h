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
	static void DrawSkybox(Shader* shader);
	static void DrawTAA();
	static void MergeSubpixels();

private:
	// 远裁剪平面上的平面网格，顶点0从左下角，依次逆时针方向排布4个顶点
	static Mesh* skyboxMesh;

	static Framebuffer* framebuffer;
	static const Eigen::Vector2f subpixelBiasX4[4];

	static Eigen::Vector2f GetSubpixelPointBias(int x, int y, int i);
	
	// 在远裁剪平面绘制一个平面网格
	static void CreateSkyboxMesh();
};

EIGEN_ALWAYS_INLINE Eigen::Vector2f Graphics::GetSubpixelPointBias(int x, int y, int subpixelIndex)
{
#if MSAA_TYPE == MSAA_X4
	return subpixelBiasX4[subpixelIndex];
#endif // MSAA_TYPE == MSAA_X4

	throw std::exception("msaaCount is unknown type.");
}

