#include "Graphics.h"
#include "Shader.h"
#include "BuiltinShader.h"

Shader *Graphics::builtinShader = new BuiltinShader();

Mesh *Graphics::skyboxMesh = nullptr;
Framebuffer *Graphics::framebuffer = nullptr;
Buffer<TAAData> *Graphics::taaBuffer = nullptr;
Camera *Graphics::camera = nullptr;
std::vector<Graphics::LightInfo> Graphics::lights = std::vector<Graphics::LightInfo>();
Eigen::Vector4f Graphics::ambientLightColor;
bool Graphics::isPerspective;

void Graphics::Clear()
{
	lights.clear();
}

void Graphics::SetCamera(Camera *camera)
{
	Graphics::camera = camera;
	isPerspective = camera->GetType() == Camera::Type::Perspective;
}

void Graphics::SetTAABuffer(Buffer<TAAData> *taabuffer)
{
	Graphics::taaBuffer = taabuffer;
}

void Graphics::SetFramebuffer(Framebuffer *framebuffer)
{
	Graphics::framebuffer = framebuffer;
}

void Graphics::SetLight(Light *light)
{
	LightInfo lightInfo = LightInfo();
	lightInfo.light = light;

	lights.push_back(lightInfo);
}

void Graphics::SetShadowMap(int lightIndex, Framebuffer *shadowMapBuffer, Camera *shadowCamera)
{
	if (lights.size() <= lightIndex)
	{
		throw new std::exception("error light index");
	}

	lights[lightIndex].shadowMapBuffer = shadowMapBuffer;
	lights[lightIndex].shadowCamera = shadowCamera;
}

void Graphics::SetDirectVisibilityMap(int lightIndex, Buffer<float> *directVisibilityMap)
{
	if (lights.size() <= lightIndex)
	{
		throw new std::exception("error light index");
	}

	lights[lightIndex].directVisibilityMap = directVisibilityMap;
}

void Graphics::SetAmbientLightColor(Eigen::Vector4f color)
{
	ambientLightColor = color;
}

void Graphics::DrawSphere(Eigen::Vector3f center, float radius, Eigen::Vector4f color, float step)
{
	for (float sita = 0; sita < 2 * M_PI; sita += step)
	{
		for (float fi = 0; fi < M_PI; fi += step)
		{
			Eigen::Vector3f pos = Eigen::Vector3f(
				center.x() + radius * cos(sita) * sin(fi),
				center.y() + radius * cos(fi),
				center.z() + radius * sin(sita) * sin(fi)
			);

			Graphics::DrawPoint(pos, color);
		}
	}
}

// TODO: 考虑子像素和深度
void Graphics::DrawPoint(Eigen::Vector3f worldPosition, Eigen::Vector4f color)
{
	Eigen::Vector4f pos;
	pos << worldPosition, 1.0f;

	pos = camera->GetFrustumMatrix() * camera->GetViewMatrix() * pos;

	if (!(pos.w() > 0 && pos.x() > -pos.w() && pos.x() < pos.w()
		&& pos.y() > -pos.w() && pos.y() < pos.w()
		&& pos.z() > -pos.w() && pos.z() < pos.w()))
	{
		return;
	}

	pos /= pos.w();

	pos.x() = (pos.x() + 1) * framebuffer->getWidth() / 2;
	pos.y() = (pos.y() + 1) * framebuffer->getHeight() / 2;


	if (pos.z() < -1)
	{
		return;
	}

	if (pos.x() < 0 || pos.x() >= framebuffer->getWidth() ||
		pos.y() < 0 || pos.y() >= framebuffer->getHeight())
	{
		return;
	}

	auto &pixelData = framebuffer->pixelBuffer.referPixel(pos.x(), pos.y());

	for (size_t subpixelIndex = 0; subpixelIndex < pixelData.subpixels.size(); subpixelIndex++)
	{
		auto &subpixel = pixelData.subpixels[subpixelIndex];
		subpixel.color = color;
		subpixel.z = pos.z();
	}
}

void Graphics::DrawMesh(const Mesh *mesh, const Eigen::Matrix4f &modelMatrix, Shader *shader, DrawFlags drawFlags)
{
	PreDrawMesh(mesh, modelMatrix, shader, drawFlags);
	DrawFullScreen();
}

void Graphics::PreDrawMesh(const Mesh *mesh, const Eigen::Matrix4f &modelMatrix, Shader *shader, DrawFlags drawFlags)
{
	EnvContext *context = EnvContextCreater::CreateEnvVariable(camera, modelMatrix);

	shader->DrawInit(context);
	builtinShader->DrawInit(context);

	appdata v;
	v2f *v2fTemp = new v2f[mesh->verticesCount];
	v2f *builtinV2fTemp = new v2f[mesh->verticesCount];

	for (size_t i = 0; i < mesh->verticesCount; i++)
	{
		v.vertex << mesh->vertices[i], 1.0f;

		if (mesh->uv0)
		{
			v.uv0 = mesh->uv0[i];
		}
		if (mesh->uv1)
		{
			v.uv1 = mesh->uv1[i];
		}
		if (mesh->uv2)
		{
			v.uv2 = mesh->uv2[i];
		}
		if (mesh->uv3)
		{
			v.uv3 = mesh->uv3[i];
		}
		if (mesh->normals)
		{
			v.normal << mesh->normals[i], 0.0f;
		}

		v2fTemp[i] = shader->vertex(v);
		assert(v2fTemp[i].vertex.w() != 0);

		builtinV2fTemp[i] = builtinShader->vertex(v);
	}

	// 遍历三角形
	for (size_t i = 0; i < mesh->edgesCount / 3; i++)
	{
		int indexes[3];
		for (size_t j = 0; j < 3; j++)
		{
			indexes[j] = mesh->edges[i * 3 + j];
		}

		// 齐次裁剪空间坐标
		bool valid = false;
		Eigen::Vector4f clipPos[3];

		// 观察空间深度，透视投影专用
		Eigen::Vector3f verticesRealZ;

		for (size_t j = 0; j < 3; j++)
		{
			clipPos[j] = v2fTemp[indexes[j]].vertex;

			if (camera->GetType() == Camera::Type::Perspective)
			{
				verticesRealZ[j] = clipPos[j].w();
			}

			float x = clipPos[j].x();
			float y = clipPos[j].y();
			float z = clipPos[j].z();
			float w = clipPos[j].w() * 1.01f;

			valid = valid ||
				(clipPos[j].w() > 0 &&
					x >= -w && x <= w &&
					y >= -w && y <= w &&
					z >= -w && z <= w);
		}

		if (!valid)
		{
			// TODO: 裁剪边界三角形
			continue;
		}

		// ndc
		Eigen::Vector4f ndcPos[3];
		for (size_t j = 0; j < 3; j++)
		{
			// 透视除法
			ndcPos[j] = clipPos[j] / clipPos[j].w();
		}

		// 屏幕坐标，z是0~1的深度
		Eigen::Vector2f screenPos[3];
		Eigen::Vector3f normalizedZ;
		for (size_t j = 0; j < 3; j++)
		{
			screenPos[j].x() = (ndcPos[j].x() + 1) * framebuffer->getWidth() / 2;
			screenPos[j].y() = (ndcPos[j].y() + 1) * framebuffer->getHeight() / 2;
			normalizedZ[j] = (ndcPos[j].z() + 1) / 2;
		}

#pragma region DEBUG_画线
		// DEBUG
		//framebuffer->drawLine(
		//	screenPosTemp[0].x(), screenPosTemp[0].y(),
		//	screenPosTemp[1].x(), screenPosTemp[1].y(),
		//	Color::LightGray);
		//framebuffer->drawLine(
		//	screenPosTemp[1].x(), screenPosTemp[1].y(),
		//	screenPosTemp[2].x(), screenPosTemp[2].y(),
		//	Color::LightGray);
		//framebuffer->drawLine(
		//	screenPosTemp[2].x(), screenPosTemp[2].y(),
		//	screenPosTemp[0].x(), screenPosTemp[0].y(),
		//	Color::LightGray);
#pragma endregion

		Eigen::Vector2i aabbMin
			= screenPos[0]
			.cwiseMin(screenPos[1])
			.cwiseMin(screenPos[2])
			.cast<int>();

		aabbMin = aabbMin.cwiseMax(Eigen::Vector2i(0, 0));

		Eigen::Vector2i aabbMax
			= screenPos[0]
			.cwiseMax(screenPos[1])
			.cwiseMax(screenPos[2])
			.cast<int>();

		aabbMax = aabbMax.cwiseMin(
			Eigen::Vector2i(framebuffer->getWidth() - 1, framebuffer->getHeight() - 1));

		for (int x = (int)aabbMin.x(); x <= (int)aabbMax.x(); x++)
		{
			for (int y = (int)aabbMin.y(); y <= (int)aabbMax.y(); y++)
			{
				auto &pixelData = framebuffer->pixelBuffer.referPixel(x, y);

				for (size_t subpixelIndex = 0; subpixelIndex < pixelData.subpixels.size(); subpixelIndex++)
				{
					auto &subpixel = pixelData.subpixels[subpixelIndex];

					Eigen::Vector2f point = subpixel.screenPosition;

					// 外部生成
					// Eigen::Vector2f point = Eigen::Vector2f(x + 0.5f, y + 0.5f);
					// point += GetSubpixelPointBias(x, y, subpixelIndex) + bias;

					if (!MathUtils::InTriangle(point, screenPos[0], screenPos[1], screenPos[2]))
					{
						continue;
					}


					Eigen::Vector3f barycentric =
						MathUtils::Barycentric(point, screenPos[0], screenPos[1], screenPos[2]);

					float z;
					float pixelRealZ; // 透视投影专用
					if (camera->GetType() == Camera::Type::Perspective)
					{
						pixelRealZ = PCI::InterpolationZ(barycentric, verticesRealZ);
						z = pixelRealZ / camera->zFar; // TODO: 更正确地映射到 0-1
					}
					else if (camera->GetType() == Camera::Type::Orthographic)
					{
						z = barycentric.dot(normalizedZ);
					}
					else
					{
						throw std::runtime_error("Camera::Type::Unknown");
					}

					if (z < 0)
					{
						continue;
					}

					if (z > subpixel.z)
					{
						continue;
					}

					if (drawFlags & DrawFlags::DrawFlags_ZBuffer)
					{
						subpixel.z = z;
					}

					if (drawFlags & DrawFlags::DrawFlags_Color)
					{
						Eigen::Matrix<float, 4, 3> verticesData;

#pragma region Custom Shader
						v2f o;
						verticesData.col(0) = clipPos[0];
						verticesData.col(1) = clipPos[1];
						verticesData.col(2) = clipPos[2];

						o.vertex = GraphicsUtils::InterpolationVector(
							barycentric, verticesRealZ, pixelRealZ, verticesData, camera->GetType());

						for (size_t i = 0; i < shader->usedTexCount; i++)
						{
							verticesData.col(0) = v2fTemp[indexes[0]].texcoords[i];
							verticesData.col(1) = v2fTemp[indexes[1]].texcoords[i];
							verticesData.col(2) = v2fTemp[indexes[2]].texcoords[i];

							o.texcoords[i] = GraphicsUtils::InterpolationVector(
								barycentric, verticesRealZ, pixelRealZ, verticesData, camera->GetType());
						}

						subpixel.v2f = o;
						subpixel.shader = shader;
#pragma endregion

#pragma region Builtin Shader
						v2f oBuiltin;

						verticesData.col(0) = builtinV2fTemp[indexes[0]].vertex;
						verticesData.col(1) = builtinV2fTemp[indexes[1]].vertex;
						verticesData.col(2) = builtinV2fTemp[indexes[2]].vertex;

						oBuiltin.vertex = GraphicsUtils::InterpolationVector(
							barycentric, verticesRealZ, pixelRealZ, verticesData, camera->GetType());

						//std::cout << oBuiltin.vertex << "\n";

						for (size_t i = 0; i < builtinShader->usedTexCount; i++)
						{
							verticesData.col(0) = builtinV2fTemp[indexes[0]].texcoords[i];
							verticesData.col(1) = builtinV2fTemp[indexes[1]].texcoords[i];
							verticesData.col(2) = builtinV2fTemp[indexes[2]].texcoords[i];

							oBuiltin.texcoords[i] = GraphicsUtils::InterpolationVector(
								barycentric, verticesRealZ, pixelRealZ, verticesData, camera->GetType());
						}

						subpixel.builtinV2f = oBuiltin;
#pragma endregion
					}
				}
			}
		}
	}

	EnvContextCreater::ClearEnvVariable(context);
}

void Graphics::DrawPostprocessing(PostprocessingPass *pass)
{
	pass->init();

	// 渲染
	for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
	{
		for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
		{
			auto &pixelData = framebuffer->pixelBuffer.referPixel(x, y);

			for (size_t subpixelIndex = 0; subpixelIndex < pixelData.subpixels.size(); subpixelIndex++)
			{
				pass->fragment(pixelData.subpixels[subpixelIndex]);
			}
		}
	}
}


void Graphics::DrawFullScreen()
{
	EnvContext *context = EnvContextCreater::CreateEnvVariable(camera, Eigen::Matrix4f::Identity());


	// 渲染
	for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
	{
		for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
		{
			auto &pixelData = framebuffer->pixelBuffer.referPixel(x, y);

			for (size_t subpixelIndex = 0; subpixelIndex < pixelData.subpixels.size(); subpixelIndex++)
			{
				auto &subpixel = pixelData.subpixels[subpixelIndex];

				if (!subpixel.shader)
				{
					continue;
				}

				context->worldPosition = subpixel.builtinV2f.vertex;
				context->screenPosition = subpixel.screenPosition;

				subpixel.shader->DrawInit(context);
				auto color = subpixel.shader->fragment(subpixel.v2f);
				subpixel.color = MathUtils::Pow(color, 1 / 2.2f);
			}
		}
	}

	// 清理 脏标记
	for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
	{
		for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
		{
			auto &pixelData = framebuffer->pixelBuffer.referPixel(x, y);

			for (size_t subpixelIndex = 0; subpixelIndex < pixelData.subpixels.size(); subpixelIndex++)
			{
				auto &subpixel = pixelData.subpixels[subpixelIndex];
				subpixel.shader = nullptr;
			}
		}
	}

	EnvContextCreater::ClearEnvVariable(context);
}

void Graphics::DrawSkybox(Shader *shader)
{
	if (skyboxMesh == nullptr)
	{
		CreateSkyboxMesh();
	}

	Eigen::Vector4f points[] = {
		Eigen::Vector4f(-1.0f, -1.0f, 1.0f, 1.0f),
		Eigen::Vector4f(1.0f, -1.0f, 1.0f, 1.0f),
		Eigen::Vector4f(1.0f, 1.0f, 1.0f, 1.0f),
		Eigen::Vector4f(-1.0f, 1.0f, 1.0f, 1.0f)
	};

	if (camera->GetType() == Camera::Type::Perspective)
	{
		for (size_t i = 0; i < 4; i++)
		{
			points[i] *= camera->zFar;
		}
	}

	auto clipToWorld = camera->GetClipToWorldMatrix();
	for (int i = 0; i < 4; i++)
	{
		skyboxMesh->vertices[i] = (clipToWorld * points[i]).head<3>();
	}

	DrawMesh(skyboxMesh, Eigen::Matrix4f::Identity(), shader, DrawFlags_ALL);
}

void Graphics::DrawTAA()
{
	for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
	{
		for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
		{
			auto &pixelData = framebuffer->pixelBuffer.referPixel(x, y);
			auto &taaData = taaBuffer->referPixel(x, y);

			for (size_t subpixelIndex = 0; subpixelIndex < MSAA_TYPE; subpixelIndex++)
			{
				auto &subpixel = pixelData.subpixels[subpixelIndex];
				auto &taaSubpixel = taaData.subpixels[subpixelIndex];

				auto sampleCount = taaSubpixel.sampleCount;

				subpixel.color = subpixel.color / float(sampleCount + 1)
					+ taaSubpixel.historyColor * (sampleCount) / float(sampleCount + 1);

				taaSubpixel.historyColor = subpixel.color;
				taaSubpixel.sampleCount++;
			}
		}
	}
}

void Graphics::MergeSubpixelsAndWrite()
{
	for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
	{
		for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
		{
			auto &pixelData = framebuffer->pixelBuffer.referPixel(x, y);

			Eigen::Vector4f colorTemp = Eigen::Vector4f::Zero();
			for (size_t subpixelIndex = 0; subpixelIndex < pixelData.subpixels.size(); subpixelIndex++)
			{
				auto &subpixel = pixelData.subpixels[subpixelIndex];
				colorTemp += subpixel.color;
			}

			colorTemp /= pixelData.subpixels.size();
			framebuffer->colorBuffer.putPixel(x, y, Color::Make(colorTemp));
		}
	}
}

float Graphics::GetDirectVisibility(Eigen::Vector2f screenPos, int lightIndex)
{
	if (lights.size() <= lightIndex)
	{
		throw new std::exception("Light index out of range");
	}

	auto lightInfo = lights[lightIndex];

	if (lightInfo.directVisibilityMap == nullptr)
	{
		return 1;
	}

	return lightInfo.directVisibilityMap->getPixel(screenPos.x(), screenPos.y());
}

// TODO
float Graphics::GetVisibility(Eigen::Vector4f worldPos, int lightIndex)
{
	if (lights.size() <= lightIndex)
	{
		throw new std::exception("Light index out of range");
	}

	auto lightInfo = lights[lightIndex];

	if (lightInfo.shadowCamera == nullptr || lightInfo.shadowMapBuffer == nullptr)
	{
		return 1;
	}

	Eigen::Vector4f viewPos = lightInfo.shadowCamera->GetViewMatrix() * worldPos;
	// TODO:
}

void Graphics::CreateSkyboxMesh()
{
	skyboxMesh = new Mesh();

	skyboxMesh->vertices = new Eigen::Vector3f[4];
	skyboxMesh->verticesCount = 4;

	skyboxMesh->edges = new int[6] {
		0, 1, 2,
			2, 3, 0
	};
	skyboxMesh->edgesCount = 6;
}

