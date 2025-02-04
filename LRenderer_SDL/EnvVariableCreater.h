#pragma once

#include "EnvVariable.h"
#include "Camera.h"

class EnvVariableCreater
{
	EnvVariableCreater() = delete;

public:
	static EnvVariable* CreateEnvVariable(const Eigen::Matrix4f& modelMatrix);
};

