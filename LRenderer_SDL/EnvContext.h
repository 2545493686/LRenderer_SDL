#pragma once

#include <vector>

#include "Eigen/Dense"

struct DirectionalLightData
{
	int lightIndex;

	// 已归一化
	Eigen::Vector4f worldSpaceDirection;
	Eigen::Vector4f color;
	float intensity;
};

class EnvContext
{
public:
	// 管线常量，初始化时更新
	float zNear;
	float zFar;

	Eigen::Matrix4f modelMatrix;
	Eigen::Matrix4f viewMatrix;
	Eigen::Matrix4f frustumMatrix;
	Eigen::Matrix4f modelToClipMatrix;
	Eigen::Matrix4f modelToViewMatrix;
	Eigen::Vector3f cameraWorldPos;
	Eigen::Matrix4f clipToWorldMatrix;

	std::vector<DirectionalLightData> directionalLightDatas;

	Eigen::Vector4f ambientLightColor;

	// 像素常量，每次片元着色器调用前更新
	Eigen::Vector4f worldPosition;
	Eigen::Vector2f screenPosition;
};