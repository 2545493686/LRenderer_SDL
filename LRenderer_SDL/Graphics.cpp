#include "Graphics.h"

Framebuffer* Graphics::framebuffer = nullptr;
Buffer<float>* Graphics::zBuffer = nullptr;

void Graphics::SetFramebuffer(Framebuffer* framebuffer)
{
	Graphics::framebuffer = framebuffer;
}

void Graphics::SetZBuffer(Buffer<float>* zBuffer)
{
	Graphics::zBuffer = zBuffer;
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
		v.uv0 = mesh->uv0[i];
		v.uv1 = mesh->uv1[i];
		v.uv2 = mesh->uv2[i];
		v.uv3 = mesh->uv3[i];

		v2fTemp[i] = shader->vertex(v);
	}

	// TODO: 计算重心坐标并插值

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
			z[j] = -clipPosTemp[j].w();
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

		//// DEBUG
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

		Eigen::Vector2f aabbMin
			= screenPosTemp[0].cwiseMin(screenPosTemp[1]).cwiseMin(screenPosTemp[2]);
		Eigen::Vector2f aabbMax
			= screenPosTemp[0].cwiseMax(screenPosTemp[1]).cwiseMax(screenPosTemp[2]);

		for (int x = (int)aabbMin.x(); x <= (int)aabbMax.x(); x++)
		{
			for (int y = (int)aabbMin.y(); y <= (int)aabbMax.y(); y++)
			{
				Eigen::Vector2f point = Eigen::Vector2f(x + 0.5f, y + 0.5f);

				if (!MathUtils::InTriangle(point, screenPosTemp[0], screenPosTemp[1], screenPosTemp[2]))
				{
					continue;
				}

				Eigen::Vector3f barycentric =
					MathUtils::Barycentric(point, screenPosTemp[0], screenPosTemp[1], screenPosTemp[2]);

				//float zt = z[0] * barycentric.x() + z[1] * barycentric.y() + z[2] * barycentric.z();
				float zt = PCI::InterpolationZ(barycentric, z);

				if (zt > zBuffer->getPixel(x, y))
				{
					continue;
				}

				zBuffer->putPixel(x, y, zt);

				// TODO: 所有属性插值
				v2f v2f;
				v2f.vertex = PCI::InterpolationVector(barycentric, z, zt, clipPosTemp);

				for (size_t i = 0; i < 4; i++)
				{
					Eigen::Vector4f texcoordTemp[3] = {
						v2fTemp[indexes[0]].texcoords[i],
						v2fTemp[indexes[1]].texcoords[i],
						v2fTemp[indexes[2]].texcoords[i],
					};

					v2f.texcoords[i] = Eigen::Vector4f(PCI::InterpolationVector(barycentric, z, zt, texcoordTemp));
				}

				framebuffer->putPixel(x, y,  Color::Make(shader->fragment(v2f)));
			}
		}
	}
}
