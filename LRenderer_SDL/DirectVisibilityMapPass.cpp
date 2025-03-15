#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>

#include "DirectVisibilityMapPass.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Light.h"

void DirectVisibilityMapPass::init()
{
	floatError = 0.00175f;
	maxBias = 3.0f / shadowCamera->zFar;
	worldToClipMatrix = shadowCamera->GetFrustumMatrix() * shadowCamera->GetViewMatrix();
}

void DirectVisibilityMapPass::fragment(SubpixelData &pixelData)
{
	auto worldPos = pixelData.builtinV2f.vertex;
	auto worldNormal = pixelData.builtinV2f.texcoords[0];
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
	float standardZ = shadowPixelData.subpixels[0].z;

	int x = static_cast<int>(pixelData.screenPosition.x());
    int y = static_cast<int>(pixelData.screenPosition.y());
	float z = ((clipPos.z() / clipPos.w()) + 1) / 2;

	assert(z > 0);
	assert(tempZ > 0);

	Eigen::Vector4f lightDir;
	if (light->GetLightType() == LightType::Directional)
	{
		lightDir = (static_cast<DirectionalLight *>(light))->direction.normalized();
	}
	else
	{
		Eigen::Vector4f cameraPos;
		cameraPos << shadowCamera->transform->position, 1.0f;
		lightDir = (worldPos - cameraPos).normalized();
	}
	float k = lightDir.dot(worldNormal);
	k = std::abs(k);
	k = 1 / k;

	float bias = std::clamp((0.85f * floatError * k), 0.0f, maxBias);

	directVisibilityMap->putPixel(x, y, (z <= (std::max(standardZ, tempZ) + bias)));
}
