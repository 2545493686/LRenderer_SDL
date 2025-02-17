#pragma once

#include "EnvVariable.h"
#include "Camera.h"
#include "Graphics.h"

class EnvVariableCreater
{
	EnvVariableCreater() = delete;

public:
	static EnvVariable* CreateEnvVariable(const Eigen::Matrix4f& modelMatrix);
	static void ClearEnvVariable(EnvVariable* context);
};

