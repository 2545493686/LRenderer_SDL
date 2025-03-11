#include "Graphics.h"

Shader* Graphics::builtinShader = new BuiltinShader();

Mesh* Graphics::skyboxMesh = nullptr;
Framebuffer* Graphics::framebuffer = nullptr;
Camera* Graphics::camera = nullptr;
Graphics::LightsList Graphics::lightsList;
Eigen::Vector4f Graphics::ambientLightColor;
bool Graphics::isPerspective;

void Graphics::SetCamera(Camera* camera)
{
	Graphics::camera = camera;
	isPerspective = camera->GetType() == Camera::Type::Perspective;
}

// 同时清楚 Graphics 状态
void Graphics::SetFramebuffer(Framebuffer* framebuffer)
{
	Graphics::framebuffer = framebuffer;

	lightsList.Clear(); 
}

void Graphics::SetLight(DirectionalLight *light)
{
	lightsList.directionalLight.push_back(light);
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


	if (pos.z() < 0)
	{
		return;
	}

	if (pos.x() < 0 || pos.x() >= framebuffer->getWidth() ||
		pos.y() < 0 || pos.y() >= framebuffer->getHeight())
	{
		return;
	}

	auto& pixelData = framebuffer->pixelBuffer.referPixel(pos.x(), pos.y());

	for (size_t subpixelIndex = 0; subpixelIndex < pixelData.subpixels.size(); subpixelIndex++)
	{
		auto& subpixel = pixelData.subpixels[subpixelIndex];
		subpixel.color = color;
		subpixel.z = pos.z();
	}
}

void Graphics::DrawMesh(const Mesh* mesh, const Eigen::Matrix4f& modelMatrix, Shader* shader)
{
	PreDrawMesh(mesh, modelMatrix, shader);
    DrawFullScreen();
}

void Graphics::PreDrawMesh(const Mesh* mesh, const Eigen::Matrix4f& modelMatrix, Shader* shader)
{
	EnvVariable* context = EnvVariableCreater::CreateEnvVariable(camera, modelMatrix);

	shader->DrawInit(context);
	builtinShader->DrawInit(context);

	appdata v;
	v2f* v2fTemp = new v2f[mesh->verticesCount];
	v2f* builtinV2fTemp = new v2f[mesh->verticesCount];

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
		float z[3];
		Eigen::Vector4f clipPos[3];
		for (size_t j = 0; j < 3; j++)
		{
			clipPos[j] = v2fTemp[indexes[j]].vertex;
			z[j] = clipPos[j].z();

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
		Eigen::Vector4f ndcTemp[3];
		for (size_t j = 0; j < 3; j++)
		{
			// 透视除法
			ndcTemp[j] = clipPos[j] / clipPos[j].w();
		}

		// 屏幕坐标
		Eigen::Vector2f screenPosTemp[3];
		for (size_t j = 0; j < 3; j++)
		{
			screenPosTemp[j].x() = (ndcTemp[j].x() + 1) * framebuffer->getWidth() / 2;
			screenPosTemp[j].y() = (ndcTemp[j].y() + 1) * framebuffer->getHeight() / 2;
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
			= screenPosTemp[0]
				.cwiseMin(screenPosTemp[1])
				.cwiseMin(screenPosTemp[2])
				.cast<int>();

		aabbMin = aabbMin.cwiseMax(Eigen::Vector2i(0, 0));

		Eigen::Vector2i aabbMax
			= screenPosTemp[0]
				.cwiseMax(screenPosTemp[1])
				.cwiseMax(screenPosTemp[2])
				.cast<int>();

		aabbMax = aabbMax.cwiseMin(
			Eigen::Vector2i(framebuffer->getWidth() - 1, framebuffer->getHeight() - 1));

		for (int x = (int)aabbMin.x(); x <= (int)aabbMax.x(); x++)
		{
			for (int y = (int)aabbMin.y(); y <= (int)aabbMax.y(); y++)
			{
				auto& pixelData = framebuffer->pixelBuffer.referPixel(x, y);

				for (size_t subpixelIndex = 0; subpixelIndex < pixelData.subpixels.size(); subpixelIndex++)
				{
					auto&subpixel = pixelData.subpixels[subpixelIndex];

					Eigen::Vector2f point = subpixel.screenPosition;

					// 外部生成
					// Eigen::Vector2f point = Eigen::Vector2f(x + 0.5f, y + 0.5f);
					// point += GetSubpixelPointBias(x, y, subpixelIndex) + bias;

					if (!MathUtils::InTriangle(point, screenPosTemp[0], screenPosTemp[1], screenPosTemp[2]))
					{
						continue;
					}


					Eigen::Vector3f barycentric =
						MathUtils::Barycentric(point, screenPosTemp[0], screenPosTemp[1], screenPosTemp[2]);

					float zt = PCI::InterpolationZ(barycentric, z, isPerspective);

					if (zt < -camera->zNear)
					{
						continue;
					}

					if (zt > subpixel.z)
					{
						continue;
					}

					subpixel.z = zt;

#pragma region Custom Shader
					v2f o;
					o.vertex = PCI::InterpolationVector(barycentric, z, zt, clipPos, isPerspective);

					for (size_t i = 0; i < shader->usedTexCount; i++)
					{
						Eigen::Vector4f texcoordTemp[3] = {
							v2fTemp[indexes[0]].texcoords[i],
							v2fTemp[indexes[1]].texcoords[i],
							v2fTemp[indexes[2]].texcoords[i],
						};

						o.texcoords[i] = PCI::InterpolationVector(barycentric, z, zt, texcoordTemp, isPerspective);
					}

					subpixel.v2f = o;
					subpixel.shader = shader;
#pragma endregion

#pragma region Builtin Shader
					v2f oBuiltin;
					Eigen::Vector4f worldPosTemp[3] = {
						builtinV2fTemp[indexes[0]].vertex,
						builtinV2fTemp[indexes[1]].vertex,
						builtinV2fTemp[indexes[2]].vertex,
					};

					oBuiltin.vertex = PCI::InterpolationVector(barycentric, z, zt, worldPosTemp, isPerspective);

					for (size_t i = 0; i < builtinShader->usedTexCount; i++)
					{
						Eigen::Vector4f texcoordTemp[3] = {
							builtinV2fTemp[indexes[0]].texcoords[i],
							builtinV2fTemp[indexes[1]].texcoords[i],
							builtinV2fTemp[indexes[2]].texcoords[i],
						};

						oBuiltin.texcoords[i] = PCI::InterpolationVector(barycentric, z, zt, texcoordTemp, isPerspective);
					}

					subpixel.builtinV2f = oBuiltin;
#pragma endregion
				}
			}
		}
	}

	EnvVariableCreater::ClearEnvVariable(context);
}

void Graphics::DrawFullScreen()
{
	// 渲染
	for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
	{
		for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
		{
			auto& pixelData = framebuffer->pixelBuffer.referPixel(x, y);

			for (size_t subpixelIndex = 0; subpixelIndex < pixelData.subpixels.size(); subpixelIndex++)
			{
				auto& subpixel = pixelData.subpixels[subpixelIndex];

				if (!subpixel.shader)
				{
					continue;
				}

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
			auto& pixelData = framebuffer->pixelBuffer.referPixel(x, y);

			for (size_t subpixelIndex = 0; subpixelIndex < pixelData.subpixels.size(); subpixelIndex++)
			{
				auto& subpixel = pixelData.subpixels[subpixelIndex];
				subpixel.shader = nullptr;
			}
		}
	}
}

void Graphics::DrawSkybox(Shader* shader)
{
	if (skyboxMesh == nullptr)
	{
		CreateSkyboxMesh();
	}

	Eigen::Vector4f points[] = {
		Eigen::Vector4f(-1.0f, -1.0f, 1.0f, 1.0f)* camera->zFar,
		Eigen::Vector4f(1.0f, -1.0f, 1.0f, 1.0f)* camera->zFar,
		Eigen::Vector4f(1.0f, 1.0f, 1.0f, 1.0f)* camera->zFar,
		Eigen::Vector4f(-1.0f, 1.0f, 1.0f, 1.0f) * camera->zFar
	};

	auto clipToWorld = camera->GetClipToWorldMatrix();
	for (int i = 0; i < 4; i++)
	{
        skyboxMesh->vertices[i] = (clipToWorld * points[i]).head<3>();
	}

	DrawMesh(skyboxMesh, Eigen::Matrix4f::Identity(), shader);
}

void Graphics::DrawTAA()
{
	for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
	{
		for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
		{
			auto& pixelData = framebuffer->pixelBuffer.referPixel(x, y);
			auto& taaData = framebuffer->taaBuffer.referPixel(x, y);

			for (size_t subpixelIndex = 0; subpixelIndex < MSAA_TYPE; subpixelIndex++)
			{
				auto& subpixel = pixelData.subpixels[subpixelIndex];
				auto& taaSubpixel = taaData.subpixels[subpixelIndex];

				auto sampleCount = taaSubpixel.sampleCount;

				subpixel.color = subpixel.color / float(sampleCount + 1)
					+ taaSubpixel.historyColor * (sampleCount) / float(sampleCount + 1);
				
				taaSubpixel.historyColor = subpixel.color;
				taaSubpixel.sampleCount++;
			}
		}
	}
}

void Graphics::MergeSubpixels()
{
	for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
	{
		for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
		{
			auto& pixelData = framebuffer->pixelBuffer.referPixel(x, y);

			Eigen::Vector4f colorTemp = Eigen::Vector4f::Zero();
			for (size_t subpixelIndex = 0; subpixelIndex < pixelData.subpixels.size(); subpixelIndex++)
			{
				auto& subpixel = pixelData.subpixels[subpixelIndex];
				colorTemp += subpixel.color;
			}

			colorTemp /= pixelData.subpixels.size();
			framebuffer->colorBuffer.putPixel(x, y, Color::Make(colorTemp));
		}
	}
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

