#include "GaussianDerivativeOperator.h"

void GaussianDerivativeOperator::Invoke(Texture *tex)
{
	auto [gaussianDerivativeX, gaussianDerivativeY] = GetGaussianDerivativeKernel(1);
	// 高斯导数核滤波
	
	auto dx = tex->Filter(gaussianDerivativeX);
	auto dy = tex->Filter(gaussianDerivativeY);

	tex->Each([&dx, &dy](int x, int y, Eigen::Vector4f &value)
	{
		auto vx = dx->ReferDirect(x, y).x();
		auto vy = dy->ReferDirect(x, y).x();

		value = Eigen::Vector4f::Ones() * std::sqrtf(vx * vx + vy * vy);
	});

	this->dx = dx;
	this->dy = dy;
}


