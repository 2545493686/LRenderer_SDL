#pragma once

#include "Eigen/Dense"


class Texture {
public:
	enum WrapMode
	{
		WrapNone,
		WrapClamp,
		WrapRepeat,
		WrapPingPong
	};

	enum FilterMode
	{
		FilterNone,
		FilterLinear,
	};

	Eigen::Vector4f* data;
	int width;
	int height;

	Texture(int width, int height) {
		this->width = width;
		this->height = height;
		data = new Eigen::Vector4f[width * height];
	}

	void PutPixel(int x, int y, Eigen::Vector4f color);

	// 使用实际坐标
	Eigen::Vector4f GetPixel(int x, int y);
	
	// 使用 uv 坐标，自动msaa或者bilinear
	// LANQ 25.2.7
	Eigen::Vector4f Sample(float x, float y, FilterMode filterMode = FilterMode::FilterNone, WrapMode wrapMode = WrapMode::WrapClamp);
	
	Texture * Copy();

	Texture * Filter(Eigen::MatrixXf kernel);

	EIGEN_ALWAYS_INLINE
	Eigen::Vector4f &ReferDirect(int x, int y) {
		return data[y * width + x];
	}

	EIGEN_ALWAYS_INLINE
	void Each(std::function<void(int x, int y, Eigen::Vector4f &value)> lambda)
	{
		for (int y = 0; y < this->height; y++)
		{
			for (int x = 0; x < this->width; x++)
			{
				lambda(x, y, ReferDirect(x, y));
			}
		}
	}
};

