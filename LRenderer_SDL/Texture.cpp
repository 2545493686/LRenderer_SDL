#include "Texture.h"

#include <cmath>

void Texture::PutPixel(int x, int y, Eigen::Vector4f color)
{
	x = std::clamp(x, 0, width - 1);
	y = std::clamp(y, 0, height - 1);
	data[y * width + x] = color;
}

Eigen::Vector4f Texture::GetPixel(int x, int y)
{
	x = std::clamp(x, 0, width - 1);
	y = std::clamp(y, 0, height - 1);
	
	return data[y * width + x];
}

Eigen::Vector4f Texture::Sample(float u, float v)
{
	// TODO: 自动msaa或者bilinear
	// TODO: WrapMode
	int x = (int)(u * (width - 1));
	int y = (int)((1 - v) * (height - 1));

	return GetPixel(x, y);
}