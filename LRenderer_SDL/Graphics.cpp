#include "Graphics.h"

Mesh* Graphics::skyboxMesh = nullptr;
Framebuffer* Graphics::framebuffer = nullptr;
Graphics::LightsList Graphics::lightsList;
Eigen::Vector4f Graphics::ambientLightColor;

const Eigen::Vector2f Graphics::subpixelBiasX4[4] = {
	Eigen::Vector2f(-0.25f, 0.25f),
    Eigen::Vector2f(0.25f, 0.25f),
    Eigen::Vector2f(-0.25f, -0.25f),
    Eigen::Vector2f(0.25f, -0.25f),
};

// 同时清楚 Graphics 状态
void Graphics::SetFramebuffer(Framebuffer* framebuffer)
{
	Graphics::framebuffer = framebuffer;

	lightsList.Clear(); 

	// 必要的初始化
	for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
	{
		for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
		{
			auto& pixelData = framebuffer->pixelBuffer.referPixel(x, y);

			for (size_t subpixelIndex = 0; subpixelIndex < MSAA_TYPE; subpixelIndex++)
			{
				auto& subpixel = pixelData.subpixels[subpixelIndex];

                subpixel.color = Color::MakeVector(Color::Black);
				subpixel.valid = false;
				subpixel.z = std::numeric_limits<float>::max();
			}
		}
	}
}

void Graphics::SetLight(DirectionalLight *light)
{
	lightsList.directionalLight.push_back(light);
}

void Graphics::SetAmbientLightColor(Eigen::Vector4f color)
{
	ambientLightColor = color;
}

void Graphics::DrawMesh(const Mesh* mesh, const Eigen::Matrix4f& modelMatrix, Shader* shader)
{
	EnvVariable* context = EnvVariableCreater::CreateEnvVariable(modelMatrix);

	shader->DrawInit(context);

	appdata v;
	v2f* v2fTemp = new v2f[mesh->verticesCount];

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
	}

	static auto provider = Random::InSquare(0.5);
	const auto bias = provider.Pop();

	// 遍历三角形
	for (size_t i = 0; i < mesh->edgesCount / 3; i++)
	{
		int indexes[3];
		for (size_t j = 0; j < 3; j++)
		{
			indexes[j] = mesh->edges[i * 3 + j];
		}

		// 齐次裁剪空间坐标
		Eigen::Vector4f clipPosTemp[3];
		float z[3];
		for (size_t j = 0; j < 3; j++)
		{
			clipPosTemp[j] = v2fTemp[indexes[j]].vertex;
			z[j] = clipPosTemp[j].w();
			//std::cout << clipPosTemp[j] << std::endl << std::endl;
		}

		// TODO: 裁剪边界三角形

		// ndc
		Eigen::Vector4f ndcTemp[3];
		for (size_t j = 0; j < 3; j++)
		{
			// 透视除法
			ndcTemp[j] = clipPosTemp[j] / clipPosTemp[j].w();
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

				for (size_t subpixelIndex = 0; subpixelIndex < MSAA_TYPE; subpixelIndex++)
				{
					Eigen::Vector2f point = Eigen::Vector2f(x + 0.5f, y + 0.5f);
					point += GetSubpixelPointBias(x, y, subpixelIndex) + bias;

					if (!MathUtils::InTriangle(point, screenPosTemp[0], screenPosTemp[1], screenPosTemp[2]))
					{
						continue;
					}


					Eigen::Vector3f barycentric =
						MathUtils::Barycentric(point, screenPosTemp[0], screenPosTemp[1], screenPosTemp[2]);

					auto&subpixel = pixelData.subpixels[subpixelIndex];

					float zt = PCI::InterpolationZ(barycentric, z);

					if (zt < 0)
					{
						continue;
					}

					if (zt > subpixel.z)
					{
						continue;
					}

					subpixel.z = zt;
					subpixel.screenPosition = point;

					// TODO: 所有属性插值
					v2f v2f;
					v2f.vertex = PCI::InterpolationVector(barycentric, z, zt, clipPosTemp);

					for (size_t i = 0; i < V2F_TEX_COUNT; i++)
					{
						Eigen::Vector4f texcoordTemp[3] = {
							v2fTemp[indexes[0]].texcoords[i],
							v2fTemp[indexes[1]].texcoords[i],
							v2fTemp[indexes[2]].texcoords[i],
						};

						v2f.texcoords[i] = Eigen::Vector4f(PCI::InterpolationVector(barycentric, z, zt, texcoordTemp));
					}

					subpixel.v2f = v2f;
					
					subpixel.valid = true;
				}
			}
		}
	}

	// 渲染
	for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
	{
		for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
		{
			auto& pixelData = framebuffer->pixelBuffer.referPixel(x, y);

			for (size_t subpixelIndex = 0; subpixelIndex < MSAA_TYPE; subpixelIndex++)
			{
				auto& subpixel = pixelData.subpixels[subpixelIndex];

				if (!subpixel.valid)
				{
					continue;
				}

				subpixel.color = shader->fragment(subpixel.v2f);
			}
		}
	}

	// 清理 脏标记
	for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
	{
		for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
		{
			auto& pixelData = framebuffer->pixelBuffer.referPixel(x, y);

            for (size_t subpixelIndex = 0; subpixelIndex < MSAA_TYPE; subpixelIndex++)
			{
				auto& subpixel = pixelData.subpixels[subpixelIndex];
				subpixel.valid = false;
			}
		}
	}

	EnvVariableCreater::ClearEnvVariable(context);
}

void Graphics::DrawSkybox(Shader* shader)
{
	if (skyboxMesh == nullptr)
	{
		CreateSkyboxMesh();
	}

	Eigen::Vector4f points[] = {
		Eigen::Vector4f(-1.0f, -1.0f, 1.0f, 1.0f) * Camera::main->zFar,
		Eigen::Vector4f(1.0f, -1.0f, 1.0f, 1.0f) * Camera::main->zFar,
		Eigen::Vector4f(1.0f, 1.0f, 1.0f, 1.0f) * Camera::main->zFar,
		Eigen::Vector4f(-1.0f, 1.0f, 1.0f, 1.0f) * Camera::main->zFar
	};

	auto clipToWorld = Camera::main->GetClipToWorldMatrix();
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

			for (size_t subpixelIndex = 0; subpixelIndex < MSAA_TYPE; subpixelIndex++)
			{
				auto& subpixel = pixelData.subpixels[subpixelIndex];

				Eigen::Vector4f colorTemp = Eigen::Vector4f::Zero();
				for (size_t subpixelIndex = 0; subpixelIndex < MSAA_TYPE; subpixelIndex++)
				{
					auto& subpixel = pixelData.subpixels[subpixelIndex];
					colorTemp += subpixel.color;
				}

				colorTemp /= MSAA_TYPE;
				framebuffer->colorBuffer.putPixel(x, y, Color::Make(colorTemp));
			}
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

