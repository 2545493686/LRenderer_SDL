#define _USE_MATH_DEFINES

#include "CannyOperator.h"

#include <cmath>

void CannyOperator::Invoke(Texture *tex)
{

}

std::pair<Eigen::MatrixXf, Eigen::MatrixXf>  CannyOperator::GetGaussianDerivativeKernel(float sigma)
{
	int size = 4 * sigma + 1;

	Eigen::MatrixXf kernelX(size, size);
	Eigen::MatrixXf kernelY(size, size);

	float sumX = 0.0f;
	float sumY = 0.0f;

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
			sumX += gX;
			sumY += gY;
		}
	}
	
	kernelX /= sumX;
	kernelY /= sumY;

	return std::make_pair(kernelX, kernelY);
}
