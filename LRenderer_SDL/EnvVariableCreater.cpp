#include "EnvVariableCreater.h"

EnvVariable* EnvVariableCreater::CreateEnvVariable(Camera *camera, const Eigen::Matrix4f& modelMatrix)
{
	EnvVariable *context = new EnvVariable;
	context->modelMatrix = modelMatrix;
	context->viewMatrix = camera->GetViewMatrix();
	context->frustumMatrix = camera->GetFrustumMatrix();

	context->modelToClipMatrix = context->frustumMatrix * context->viewMatrix * context->modelMatrix;

	context->cameraWorldPos = camera->transform->position;

	auto& lights = Graphics::lightsList;
	context->directionalLightCount = lights.directionalLight.size();
	context->directionalLightDatas = new DirectionalLightLightData[context->directionalLightCount];
	
	for (size_t i = 0; i < context->directionalLightCount; i++)
	{
		auto& data = context->directionalLightDatas[i];
		auto& light = lights.directionalLight[i];
		
		data.color = light->color;
		data.worldSpaceDirection = light->direction.normalized();
		data.intensity = light->intensity;
	}

	context->ambientLightColor = Graphics::ambientLightColor;

	return context;
}

void EnvVariableCreater::ClearEnvVariable(EnvVariable* context)
{
	delete context->directionalLightDatas;
	delete context;
}
