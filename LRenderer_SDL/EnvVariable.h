#pragma once

#include "Eigen/Dense"

struct DirectionalLightLightData
{
	// 已归一化
	Eigen::Vector4f worldSpaceDirection;
	Eigen::Vector4f color;
	float intensity;
};

class EnvVariable
{
public:
	Eigen::Matrix4f modelMatrix;
	Eigen::Matrix4f viewMatrix;
	Eigen::Matrix4f frustumMatrix;
	Eigen::Matrix4f modelToClipMatrix;
	Eigen::Vector3f cameraWorldPos;

	DirectionalLightLightData* directionalLightDatas;
	int directionalLightCount;
};