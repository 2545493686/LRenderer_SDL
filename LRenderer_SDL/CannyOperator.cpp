#define _USE_MATH_DEFINES

#include "CannyOperator.h"

#include <stack>
#include <cmath>

void CannyOperator::Invoke(Texture *tex)
{
	auto [gaussianDerivativeX, gaussianDerivativeY] = GetGaussianDerivativeKernel(1);

	// 灰度化
	tex->Each([](int x, int y, Eigen::Vector4f &value) 
	{
		value = Eigen::Vector4f::Ones() * (value.x() * 0.299f + value.y() * 0.587f + value.z() * 0.114f);
	});

	// 高斯导数核滤波
	auto dx = tex->Filter(gaussianDerivativeX);
	auto dy = tex->Filter(gaussianDerivativeY);

	tex->Each([&dx, &dy](int x, int y, Eigen::Vector4f &value)
	{
		auto vx = dx->ReferDirect(x, y).x();
		auto vy = dy->ReferDirect(x, y).x();

		value = Eigen::Vector4f::Ones() * std::sqrtf(vx * vx + vy * vy);
	});

	// 非最大值抑制
	Eigen::MatrixXf ordinary(1, 1);
	ordinary.fill(1);

	auto temp = tex->Copy();
	tex->Each([&dx, &dy, &temp](int x, int y, Eigen::Vector4f &value)
	{
		auto vx = dx->ReferDirect(x, y).x();
		auto vy = dy->ReferDirect(x, y).x();

		Eigen::Vector2f gradient = Eigen::Vector2f(vx, vy).normalized();
		
		auto u = (temp->width - x - gradient.x()) / temp->width;
		auto v = (temp->height - y - gradient.y()) / temp->height;
		auto v1 = temp->Sample(u, v, Texture::FilterLinear, Texture::WrapPingPong);

		gradient *= -1;
		u = (temp->width - x - gradient.x()) / temp->width;
		v = (temp->height - y - gradient.y()) / temp->height;
		auto v2 = temp->Sample(u, v, Texture::FilterLinear, Texture::WrapPingPong);

		value = (value.x() > v1.x() && value.x() > v2.x()) ? value : Eigen::Vector4f::Zero();
	});

	// 双门限
	Eigen::MatrixXi hignThreshold(tex->width, tex->height);
	Eigen::MatrixXi lowThreshold(tex->width, tex->height);

	float maxVal = tex->ReferDirect(0, 0).x();
	tex->Each([&maxVal](int x, int y, Eigen::Vector4f &value) {
		maxVal = std::max(maxVal, value.x());
	});

	tex->Each([&hignThreshold, &lowThreshold, &maxVal](int x, int y, Eigen::Vector4f &value)
	{
		float v = value.x();
		hignThreshold(x, y) = v > (maxVal * 0.3f);
		lowThreshold(x, y) = v > (maxVal * 0.1f);
	});
	
	// 连接边
	auto visitStack = std::stack<std::pair<int, int>>();

	tex->Each([&hignThreshold, &lowThreshold, &visitStack, &tex, &dx, &dy](int texX, int texY)
	{
		if (hignThreshold(texX, texY))
		{
			visitStack.push(std::make_pair(texX, texY));
		}

		while (!visitStack.empty())
		{
			auto [x, y] = visitStack.top();
			visitStack.pop();
			
			auto vdx = dx->ReferDirect(x, y).x();
			auto vdy = dy->ReferDirect(x, y).x();

			for (int i = -1; i <= 1; i++)
			{
				for (int j = -1; j <= 1; j++)
				{
					int sx = x + i;
					int sy = y + j;

					tex->LegalizationCoordinates(sx, sy, Texture::WrapPingPong);
					
					if (lowThreshold(sx, sy) && !hignThreshold(sx, sy))
					{
						hignThreshold(sx, sy) = 1;
						visitStack.push(std::make_pair(sx, sy));
					}
				}
			}
		}
	});

	tex->Each([&hignThreshold, &lowThreshold](int x, int y, Eigen::Vector4f &value)
	{
		value = Eigen::Vector4f::Ones() * hignThreshold(x, y);
	});

	delete dx;
	delete dy;
	delete temp;
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
