#include "EnvVariableCreater.h"

EnvVariable* EnvVariableCreater::CreateEnvVariable(const Eigen::Matrix4f& modelMatrix)
{
	EnvVariable *context = new EnvVariable;
	context->modelMatrix = modelMatrix;
	context->viewMatrix = Camera::main->GetViewMatrix();
	context->frustumMatrix = Camera::main->GetFrustumMatrix();

	context->modelToClipMatrix = context->frustumMatrix * context->viewMatrix * context->modelMatrix;

	context->cameraWorldPos = Camera::main->transform->position;

	return context;
}
