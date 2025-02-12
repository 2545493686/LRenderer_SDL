#pragma once
#include "Mesh.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "PerspectiveCorrectInterpolation.h"
#include "Random.h"
#include "spdlog/spdlog.h"

constexpr auto MSAA_X4 = 4;
constexpr auto MSAA_TYPE = MSAA_X4;

class Graphics
{
public:
	struct SubpixelData
	{
		Eigen::Vector4f color;
		uint8_t sampleCount;
		float z;

		Eigen::Vector2f screenPosition;

		// TODO: 运动向量
		Eigen::Vector4f worldPosition;
		Eigen::Vector2f vectorMotion;

		v2f v2f;
	};

	struct PixelData
	{
		// TODO: 锚点颜色
		uint16_t sampleCount;
		Eigen::Vector4f anchorColor;

		SubpixelData subpixels[MSAA_TYPE];
	};

	// 输出
	static void SetFramebuffer(Framebuffer* framebuffer);
	static void DrawMesh(const Mesh* mesh, const Eigen::Matrix4f& modelMatrix, Shader* shader);

private:
	static Framebuffer* framebuffer;
	static Buffer<PixelData>* gBuffer;

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


