#include "EnvContextCreater.h"
#include "Graphics.h"

EnvContext *EnvContextCreater::CreateEnvVariable(Camera *camera, const Eigen::Matrix4f &modelMatrix)
{
	EnvContext *context = new EnvContext;
	context->modelMatrix = modelMatrix;
	context->viewMatrix = camera->GetViewMatrix();
	context->frustumMatrix = camera->GetFrustumMatrix();

	context->modelToClipMatrix = context->frustumMatrix * context->viewMatrix * context->modelMatrix;
	context->modelToViewMatrix = context->viewMatrix * context->modelMatrix;
	context->clipToWorldMatrix = (context->frustumMatrix * context->viewMatrix).inverse();

	context->cameraWorldPos << camera->transform->position, 1.0f;

	context->zNear = camera->zNear;
	context->zFar = camera->zFar;

	auto &lights = Graphics::lights;
	context->directionalLightDatas.resize(0);

	for (size_t i = 0; i < Graphics::lights.size(); i++)
	{
		auto &lightInfo = lights[i];

		if (lightInfo.light->GetLightType() == LightType::Directional)
		{
			DirectionalLight *light = static_cast<DirectionalLight *>(lightInfo.light);

			DirectionalLightData data;
			data.color = light->color;
			data.worldSpaceDirection = light->direction.normalized();
			data.intensity = light->intensity;
			data.lightIndex = i;
			
			context->directionalLightDatas.push_back(data);
		}
		else
		{
			throw std::runtime_error("Light type not supported");
		}
	}

	context->ambientLightColor = Graphics::ambientLightColor;

	return context;
}

void EnvContextCreater::ClearEnvVariable(EnvContext *context)
{
	delete context;
}
