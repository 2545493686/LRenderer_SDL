#pragma once
#define _USE_MATH_DEFINES
#include <cmath>

#include "spdlog/spdlog.h"

#include "Mesh.h"
#include "Framebuffer.h"
#include "PerspectiveCorrectInterpolation.h"
#include "Random.h"
#include "GraphicsSettings.h"
#include "Light.h"
#include "Camera.h"
#include "EnvContextCreater.h"
#include "GraphicsUtils.h"
#include "PostprocessingPass.h"

class Framebuffer;
class Shader;
class BuiltinShader;

enum DrawFlags
{
	DrawFlags_None = 0,
	DrawFlags_ZBuffer = 1 << 0,
	DrawFlags_Color = 1 << 1,
	DrawFlags_ALL = (1 << 2) - 1
};

class Graphics
{
	friend class EnvContextCreater;

public:
	static void Clear();

	static void SetCamera(Camera *camera);
	static void SetFramebuffer(Framebuffer *framebuffer);
	static void SetTAABuffer(Buffer<TAAData> *taabuffer);
	static void SetLight(Light *light);
	static void SetShadowMap(int lightIndex, Framebuffer *shadowMapBuffer, Camera *shadowCamera);
	static void SetDirectVisibilityMap(int lightIndex, Buffer<float> *directVisibilityMap);

	static void SetAmbientLightColor(Eigen::Vector4f color);

	static void DrawSphere(Eigen::Vector3f center, float radius, Eigen::Vector4f color, float step = 0.1f);
	static void DrawPoint(Eigen::Vector3f worldPosition, Eigen::Vector4f color);
	static void DrawMesh(const Mesh *mesh, const Eigen::Matrix4f &modelMatrix, Shader *shader, DrawFlags drawFlags);
	static void PreDrawMesh(const Mesh *mesh, const Eigen::Matrix4f &modelMatrix, Shader *shader, DrawFlags drawFlags);
	static void DrawPostprocessing(PostprocessingPass *pass);
	static void DrawFullScreen();

	static void DrawSkybox(Shader *shader);
	static void DrawTAA();
	static void MergeSubpixelsAndWrite();

	// lightIndex 根据 SetLight 的调用而定。只有对该 index 调用 SetShadowMap 后才会返回阴影值。
	static float GetVisibility(Eigen::Vector4f worldPos, int lightIndex);
	
	static float GetDirectVisibility(Eigen::Vector2f screenPos, int lightIndex);

private:
	struct LightInfo
	{
		Light *light; 
		
		// 可能为空
		Framebuffer *shadowMapBuffer;
		Camera *shadowCamera;

		Buffer<float> *directVisibilityMap;

		LightInfo()
		{
			light = nullptr;
			shadowMapBuffer = nullptr;
			shadowCamera = nullptr;
			directVisibilityMap = nullptr;
		}
	};

	static Shader *builtinShader;
	static Eigen::Vector4f ambientLightColor;

	// 远裁剪平面上的平面网格，顶点0从左下角，依次逆时针方向排布4个顶点
	static Mesh *skyboxMesh;

	static Framebuffer *framebuffer;
	static Buffer<TAAData> *taaBuffer;

	static Camera *camera;
	static bool isPerspective;

	static std::vector<LightInfo> lights;

	// 在远裁剪平面绘制一个平面网格
	static void CreateSkyboxMesh();
};



