#define _USE_MATH_DEFINES

#include "CannyOperator.h"

#include <cmath>

void CannyOperator::Invoke(Texture *tex)
{
	auto [gaussianDerivativeX, gaussianDerivativeY] = GetGaussianDerivativeKernel(1);
	
	for (size_t y = 0; y < tex->height; y++)
	{
		for (size_t x = 0; x < tex->width; x++)
		{
			auto color = tex->data[y * tex->width + x];
			tex->data[y * tex->width + x] = Eigen::Vector4f::Ones() * (color.x() * 0.299f + color.y() * 0.587f + color.z() * 0.114f);
		}
	}

	auto dx = tex->Filter(gaussianDerivativeX);
	auto dy = tex->Filter(gaussianDerivativeY);

	for (size_t y = 0; y < tex->height; y++)
	{
		for (size_t x = 0; x < tex->width; x++)
		{
			auto vx = dx->data[y * tex->width + x].x();
			auto vy = dy->data[y * tex->width + x].x();

			tex->data[y * tex->width + x] = Eigen::Vector4f::Ones() * std::sqrtf(vx * vx + vy * vy);
		}
	}
}

EIGEN_ALWAYS_INLINE
std::pair<Eigen::MatrixXf, Eigen::MatrixXf>  CannyOperator::GetGaussianDerivativeKernel(float sigma)
{
	int size = 4 * sigma + 1;

	Eigen::MatrixXf kernelX(size, size);
	Eigen::MatrixXf kernelY(size, size);

	for (int x = 0; x < size; x++)
	{
		for (int y = 0; y < size; y++)
		{
			int xPos = x - size / 2;
			int yPos = y - size / 2;

			float gX = (-xPos / (2 * M_PI * std::powf(sigma, 4))) * exp(-(xPos * xPos + yPos * yPos) / (2 * sigma * sigma));
			float gY = (-yPos / (2 * M_PI * std::powf(sigma, 4))) * exp(-(xPos * xPos + yPos * yPos) / (2 * sigma * sigma));
			
			kernelX(x, y) = gX;
			kernelY(x, y) = gY;
		}
	}
	
	return std::make_pair(kernelX, kernelY);
}
