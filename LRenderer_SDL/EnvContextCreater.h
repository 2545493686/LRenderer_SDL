#pragma once

#include "EnvContext.h"
#include "Camera.h"

class EnvContextCreater
{
	EnvContextCreater() = delete;

public:
	static EnvContext* CreateEnvVariable(Camera* camera, const Eigen::Matrix4f& modelMatrix);
	static void ClearEnvVariable(EnvContext* context);
};

