#include "InitShadowmapPass.h"

void InitShadowMapPass::init()
{
	worldToClipMatrix = camera->GetFrustumMatrix() * camera->GetViewMatrix();
}

void InitShadowMapPass::fragment(SubpixelData& pixelData)
{
	pixelData.tempData = 0;

	// 生成阴影相机下的uv
	auto worldPos = pixelData.builtinV2f.vertex;
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
	Eigen::Vector2f uv;
	uv.x() = (ndc.x() + 1) * shadowMap->getWidth() / 2;
    uv.y() = (ndc.y() + 1) * shadowMap->getHeight() / 2;

	auto&shadowPixelData = shadowMap->pixelBuffer.referPixel(uv.x(), uv.y());
	
	SubpixelData subpixel = SubpixelData(uv);
	shadowPixelData.subpixels.push_back(subpixel);
	
	pixelData.tempData = shadowPixelData.subpixels.size() - 1;
}
