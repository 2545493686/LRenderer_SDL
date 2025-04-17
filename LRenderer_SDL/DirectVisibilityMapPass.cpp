#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>

#include "DirectVisibilityMapPass.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Light.h"
#include "Random.h"
#include "Time.h"

void DirectVisibilityMapPass::init()
{
	floatError = 0.00175f;
	maxBias = 3.0f / shadowCamera->zFar;
	
	worldToClipMatrix = shadowCamera->GetFrustumMatrix() * shadowCamera->GetViewMatrix();
	viewToWorld = shadowCamera->GetViewMatrix().inverse();

	twoPowSampleCount = std::pow(2, raySampleCount - 1) - 1;

	lightType = light->GetLightType();

	if (lightType == LightType::Directional)
	{
		DirectionalLight *directionalLight = static_cast<DirectionalLight *>(light);
		minRadius = directionalLight->sunRadius / std::powf(2, rayCount);
		maxRadius = directionalLight->sunRadius;
		directRatio = std::powf(minRadius / maxRadius, 2);
	}
}

// 为假时 distance 被赋值为距离
bool DirectVisibilityMapPass::TestPointVisibility(const Eigen::Vector4f &worldPos, float &distance)
{
	Eigen::Vector4f clipPos = worldToClipMatrix * worldPos;

	float w = clipPos.w() * 1.01f;
	if (w <= 0)
	{
		return 1;
	}

	for (size_t i = 0; i < 3; i++)
	{
		if (clipPos[i] < -w || clipPos[i] > w)
		{
			return 1;
		}
	}

	Eigen::Vector4f ndc = clipPos / clipPos.w();
	Eigen::Vector2f uv; // shadow map uv

	uv.x() = (ndc.x() + 1) * shadowMapBuffer->getWidth() / 2;
	uv.y() = (ndc.y() + 1) * shadowMapBuffer->getHeight() / 2;

	auto &shadowPixelData = shadowMapBuffer->pixelBuffer.referPixel(uv.x(), uv.y());
	float standardZ = shadowPixelData.subpixels[0].z;

	float z = ((clipPos.z() / clipPos.w()) + 1) / 2;
	distance = z - standardZ;
	return z <= standardZ;
}

// ray 需要已经是正则化的
bool DirectVisibilityMapPass::TestRayVisibility(const Eigen::Vector4f &shaderPointWorldPos, const Eigen::Vector4f &ray, float basicBias)
{
	float cameraDistance = (shadowCamera->transform->position - shaderPointWorldPos.head<3>()).norm() - basicBias;
	float step = cameraDistance / (twoPowSampleCount);

	float dis;
	if (!TestPointVisibility(shaderPointWorldPos + ray * basicBias, dis))
	{
		if (!TestPointVisibility(shaderPointWorldPos + ray * (basicBias + dis * 2 / 3), dis))
		{
			return 0;
		}
	}

	for (size_t i = 0; i < raySampleCount - 1; i++)
	{
		if (!TestPointVisibility(shaderPointWorldPos + ray * step, dis))
		{

			if (!TestPointVisibility(shaderPointWorldPos + ray * (step + dis * 2 / 3), dis))
			{
				return 0;
			}
		}
		
		step *= 2;
	}

	return 1;
}

void DirectVisibilityMapPass::fragment(SubpixelData &pixelData)
{
	auto worldPos = pixelData.builtinV2f.vertex;
	auto worldNormal = pixelData.builtinV2f.texcoords[0];

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
	//float tempZ = shadowPixelData.subpixels[pixelData.tempData].z;
	float standardZ = shadowPixelData.subpixels[0].z;

	int x = static_cast<int>(pixelData.screenPosition.x());
    int y = static_cast<int>(pixelData.screenPosition.y());
	float z = ((clipPos.z() / clipPos.w()) + 1) / 2;

	assert(z > 0);
	assert(standardZ > 0);

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

	float bias = std::clamp((0.65f * floatError * k), 0.0f, maxBias);
	
	int directVisibility = z <= (standardZ + bias); //高精度采样
	directVisibility *= directRatio;

	double visibility = 0;
	int visibilityCount = 0;
	double visibilityK = 0;

	DirectionalLight *directionalLight = static_cast<DirectionalLight *>(light);
	Eigen::Vector4f sunCenter = -lightDir * (directionalLight->sunDistance / 15);
	sunCenter.w() = 1;

	static auto randomProvider = Random::InRange(0, 1);

	// 生成光线
	if (lightType == LightType::Directional)
	{
		float sitaStep = 2 * M_PI / (rayCount);
		float rStep = (maxRadius - minRadius) / rayCount;

		for (size_t i = 0; i < rayCount; i++)
		{
			float sita = sitaStep * i + randomProvider.Pop() * sitaStep;
			
			float random = randomProvider.Pop();
			random = 1 - sqrt(1 - random);
			//random = sqrt(random);
			
			float r = minRadius + (random) * (maxRadius - minRadius);

			Eigen::Vector4f rayBias = Eigen::Vector4f::Zero();
			rayBias.x() = r * std::cosf(sita);
            rayBias.y() = r * std::sinf(sita);

			rayBias = viewToWorld * rayBias;

			Eigen::Vector4f ray = ((sunCenter + rayBias) - worldPos);

			float rayVisibbility = TestRayVisibility(worldPos, ray.normalized(), bias * 50.0f);
			float c = std::max(lightDir.dot(-ray), 0.0f) / ray.norm();
			visibilityK += c;
			visibility += rayVisibbility * c;
			visibilityCount++;
		}
	}

	directVisibilityMap->putPixel(x, y, 
		directVisibility + static_cast<float>(visibility) / visibilityK);
}
