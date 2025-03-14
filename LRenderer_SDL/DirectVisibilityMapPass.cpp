#include "DirectVisibilityMapPass.h"
#include "Camera.h"
#include "Framebuffer.h"

void DirectVisibilityMapPass::init()
{
	worldToClipMatrix = shadowCamera->GetFrustumMatrix() * shadowCamera->GetViewMatrix();
}

void DirectVisibilityMapPass::fragment(SubpixelData &pixelData)
{
	auto worldPos = pixelData.builtinV2f.vertex;
	//std::cout << "worldPos: " << worldPos << std::endl;

	Eigen::Vector4f clipPos = worldToClipMatrix * worldPos;

	float w = clipPos.w() * 1.01f;
	if (w <= 0)
	{
		return;
	}

	for (size_t i = 0; i < 3; i++)
	{
		if (clipPos[i] < -w || clipPos[i] > w)
		{
			return;
		}
	}

	Eigen::Vector4f ndc = clipPos / clipPos.w();
	Eigen::Vector2f uv; // shadow map uv

	uv.x() = (ndc.x() + 1) * shadowMapBuffer->getWidth() / 2;
	uv.y() = (ndc.y() + 1) * shadowMapBuffer->getHeight() / 2;

	auto &shadowPixelData = shadowMapBuffer->pixelBuffer.referPixel(uv.x(), uv.y());
	float tempZ = shadowPixelData.subpixels[pixelData.tempData].z;
	
	int x = static_cast<int>(pixelData.screenPosition.x());
    int y = static_cast<int>(pixelData.screenPosition.y());
	float z = ((clipPos.z() / clipPos.w()) + 1) / 2;

	//std::cout << "z: " << z << std::endl;
	//std::cout << "temp z: " << tempZ << std::endl;

	directVisibilityMap->putPixel(x, y, z < (tempZ + 0.001f));
}
