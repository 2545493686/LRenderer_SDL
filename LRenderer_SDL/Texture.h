#pragma once

#include "Eigen/Dense"


class Texture {
	enum WrapMode
	{
		WrapNone,
		WrapClamp,
		WrapRepeat,
	};

	enum FilterMode
	{
		FilterNone,
		FilterLinear,
	};

public:
	WrapMode wrapMode = Texture::WrapMode::WrapRepeat;
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
	Eigen::Vector4f Sample(float x, float y);
	
	Texture * Filter(Eigen::MatrixXf kernel);
};

