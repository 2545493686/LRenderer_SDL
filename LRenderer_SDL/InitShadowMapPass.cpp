#include "InitShadowmapPass.h"

void InitShadowMapPass::init()
{
	worldToClipMatrix = camera->GetFrustumMatrix() * camera->GetViewMatrix();
}

void InitShadowMapPass::fragment(SubpixelData& pixelData)
{
	// 生成阴影相机下的uv
	auto worldPos = pixelData.builtinV2f.texcoords[0];
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

	int x = MathUtils::Clamp(static_cast<int>(uv.x()), 0, shadowMap->getWidth() - 1);
	int y = MathUtils::Clamp(static_cast<int>(uv.y()), 0, shadowMap->getHeight() - 1);

	auto&shadowPixelData = shadowMap->pixelBuffer.referPixel(x, y);
	
	SubpixelData subpixel = SubpixelData(uv);
	shadowPixelData.subpixels.push_back(subpixel);
	
	pixelData.shadowSubIndex = shadowPixelData.subpixels.size() - 1;
}
