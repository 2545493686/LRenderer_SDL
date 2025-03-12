#pragma once
#define _USE_MATH_DEFINES
#include <cmath>

#include "Mesh.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "PerspectiveCorrectInterpolation.h"
#include "Random.h"
#include "spdlog/spdlog.h"
#include "GraphicsSettings.h"
#include "DirectionalLight.h"
#include "Camera.h"
#include "EnvVariableCreater.h"
#include "BuiltinShader.h"
#include "GraphicsUtils.h"

class Graphics
{
	friend class EnvVariableCreater;

public:
	static void Clear();

	static void SetCamera(Camera* camera);
	static void SetFramebuffer(Framebuffer *framebuffer);
	static void SetLight(DirectionalLight *light);
	static void SetAmbientLightColor(Eigen::Vector4f color);

	static void DrawSphere(Eigen::Vector3f center, float radius, Eigen::Vector4f color, float step = 0.1f);
	static void DrawPoint(Eigen::Vector3f worldPosition, Eigen::Vector4f color);
	static void DrawMesh(const Mesh* mesh, const Eigen::Matrix4f& modelMatrix, Shader* shader);
	static void PreDrawMesh(const Mesh* mesh, const Eigen::Matrix4f& modelMatrix, Shader* shader);
	static void DrawFullScreen();

	static void DrawSkybox(Shader* shader);
	static void DrawTAA();
	static void MergeSubpixelsAndOutput();

private:
	struct LightsList
	{
		std::vector<DirectionalLight *> directionalLight;

	public:
		void Clear()
		{
			directionalLight.clear();
		}
	};

	static Shader *builtinShader;
	static Eigen::Vector4f ambientLightColor;

	// 远裁剪平面上的平面网格，顶点0从左下角，依次逆时针方向排布4个顶点
	static Mesh* skyboxMesh;

	static Framebuffer* framebuffer;
	static Camera* camera;
	static bool isPerspective;

	static LightsList lightsList;

	// 在远裁剪平面绘制一个平面网格
	static void CreateSkyboxMesh();
};



