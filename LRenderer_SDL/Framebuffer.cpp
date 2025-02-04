#include "Framebuffer.h"

Framebuffer::Framebuffer(int width, int height)
{
	this->width = width;
	this->height = height;
	this->frameBuffer.resize(width * height, 0);
}


void Framebuffer::clear(uint32_t color)
{
	std::fill(frameBuffer.begin(), frameBuffer.end(), color);
}

void Framebuffer::putPixel(int x, int y, uint32_t color)
{
	if (x >= 0 && x < width && y >= 0 && y < height) {
		frameBuffer[y * width + x] = color;
	}
}

// DDA算法绘制直线 TODO：Bresenham
void Framebuffer::drawLine(int x0, int y0, int x1, int y1, uint32_t color)
{
	float k = (y1 - y0) / (float)(x1 - x0);
	if (k >= -1 && k <= 1) {
		if (x0 > x1) {
			std::swap(x0, x1);
			std::swap(y0, y1);
		}
		float y = y0;
		for (int x = x0; x <= x1; ++x) {
			putPixel(x, y, color);
			y += k;
		}
	}
	else {
		if (y0 > y1) {
			std::swap(x0, x1);
			std::swap(y0, y1);
		}
		k = 1 / k;
		float x = x0;
		for (int y = y0; y <= y1; ++y) {
			putPixel(x, y, color);
			x += k;
		}
	}
}

void Framebuffer::drawLine(Eigen::Vector2f p0, Eigen::Vector2f p1, uint32_t color)
{
	drawLine(p0.x(), p0.y(), p1.x(), p1.y(), color);
}

void Framebuffer::drawMesh(const Mesh *mesh, const Eigen::Matrix4f &modelMatrix, Shader *shader)
{
	EnvVariable *context = EnvVariableCreater::CreateEnvVariable(modelMatrix);

	shader->DrawInit(context);

	appdata v;
    v2f** v2fTemp = new v2f*[mesh->verticesCount];

	for (size_t i = 0; i < mesh->verticesCount; i++)
	{
		v.vertex << mesh->vertices[i], 1.0f;
		v2fTemp[i] = shader->vertex(&v);
	}

	// TODO: 计算重心坐标并插值

	// 透视除法
	for (size_t i = 0; i < mesh->verticesCount; i++)
	{
		v2fTemp[i]->vertex /= v2fTemp[i]->vertex.w();
	}

	// 转换到屏幕坐标
	for (size_t i = 0; i < mesh->verticesCount; i++)
	{
		v2fTemp[i]->vertex.x() = (v2fTemp[i]->vertex.x() + 1) * width / 2;
		v2fTemp[i]->vertex.y() = (v2fTemp[i]->vertex.y() + 1) * height / 2;
	}

	// 打印mesh顶点数量
	for (size_t i = 1; i < mesh->edgesCount / 3; i++)
	{
		drawLine(
			v2fTemp[mesh->edges[i * 3]]->vertex.x(), v2fTemp[mesh->edges[i * 3]]->vertex.y(),
			v2fTemp[mesh->edges[i * 3 + 1]]->vertex.x(), v2fTemp[mesh->edges[i * 3 + 1]]->vertex.y(),
			0xFFFFFFFF);
		drawLine(
			v2fTemp[mesh->edges[i * 3 + 1]]->vertex.x(), v2fTemp[mesh->edges[i * 3 + 1]]->vertex.y(),
			v2fTemp[mesh->edges[i * 3 + 2]]->vertex.x(), v2fTemp[mesh->edges[i * 3 + 2]]->vertex.y(),
			0xFFFFFFFF);
		drawLine(
			v2fTemp[mesh->edges[i * 3 + 2]]->vertex.x(), v2fTemp[mesh->edges[i * 3 + 2]]->vertex.y(),
			v2fTemp[mesh->edges[i * 3]]->vertex.x(), v2fTemp[mesh->edges[i * 3]]->vertex.y(),
			0xFFFFFFFF);
	}
}



