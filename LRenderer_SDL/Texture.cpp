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
	int x = (int)((1 - u) * (width - 1));
	int y = (int)((1 - v) * (height - 1));

	return GetPixel(x, y);
}

Texture * Texture::Filter(Eigen::MatrixXf kernel)
{
	Texture *tex = new Texture(width, height);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			Eigen::Vector4f sum = Eigen::Vector4f::Zero();

			for (int kernelY = 0; kernelY < kernel.rows(); kernelY++)
			{
				for (int kernelX = 0; kernelX < kernel.cols(); kernelX++)
				{
					int biasX = kernelX - kernel.cols() / 2;
					int biasY = kernelY - kernel.rows() / 2;

					int valueX = std::abs(x + biasX);
					int valueY = std::abs(y + biasY);

					valueX = (width - 1) - std::abs((width - 1) - valueX);
					valueY = (height - 1) - std::abs((height - 1) - valueY);

					sum += kernel(kernelY, kernelX) * data[valueY * width + valueX];
				}
			}

			tex->data[y * width + x] = sum;
		}
	}

	return tex;
}