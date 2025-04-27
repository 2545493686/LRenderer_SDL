#pragma once
#define _USE_MATH_DEFINES

#include "TextureOperator.h"
#include <cmath>

class GaussianDerivativeOperator : public TextureOperator
{
public:
	// output
	Texture *dx;
	Texture *dy;

	~GaussianDerivativeOperator()
	{
		if (dx)
		{
			delete dx;
		}
		if (dy)
		{
			delete dy;
		}
	}

	// Í¨¹ý TextureOperator ¼Ì³Ð
	void Invoke(Texture *tex) override;
	EIGEN_ALWAYS_INLINE std::pair<Eigen::MatrixXf, Eigen::MatrixXf> 
	GetGaussianDerivativeKernel(float sigma)
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
};

