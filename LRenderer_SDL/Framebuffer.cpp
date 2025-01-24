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




