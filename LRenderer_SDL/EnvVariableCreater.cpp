#include "EnvVariableCreater.h"

EnvVariable* EnvVariableCreater::CreateEnvVariable(const Eigen::Matrix4f& modelMatrix)
{
	EnvVariable *context = new EnvVariable;
	context->modelMatrix = modelMatrix;
	context->viewMatrix = Camera::main->GetViewMatrix();
	context->frustumMatrix = Camera::main->GetFrustumMatrix();

	context->modelToClipMatrix = context->frustumMatrix * context->viewMatrix * context->modelMatrix;

	context->cameraWorldPos = Camera::main->transform->position;

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

	return context;
}

void EnvVariableCreater::ClearEnvVariable(EnvVariable* context)
{
	delete context->directionalLightDatas;
	delete context;
}
