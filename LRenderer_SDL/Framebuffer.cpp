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

void Framebuffer::drawMesh(Mesh mesh)
{
	
}



