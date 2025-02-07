#include "Texture.h"

void Texture::PutPixel(int x, int y, Eigen::Vector4f color)
{
	if (x >= 0 && x < width && y >= 0 && y < height) {
		data[y * width + x] = color;
	}
}

Eigen::Vector4f Texture::GetPixel(int x, int y)
{
	if (x >= 0 && x < width && y >= 0 && y < height) {
		return data[y * width + x];
	}

	return Eigen::Vector4f(0, 0, 0, 0); // Return default value if out of bounds
}

Eigen::Vector4f Texture::Sample(float u, float v)
{
	// TODO: 自动msaa或者bilinear
	int x = (int)(u * width);
	int y = (int)(v * height);

	return GetPixel(x, y);
}