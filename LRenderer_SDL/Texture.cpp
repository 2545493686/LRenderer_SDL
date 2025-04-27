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

// gpt o4 mini high 编写
void Texture::LegalizationCoordinates(int &x, int &y, WrapMode wrapMode)
{
    // 处理 x 和 y 像素坐标包裹逻辑
    switch (wrapMode) {
    case WrapMode::WrapNone:
        // 如果坐标超出范围，返回零
        if (x < 0 || x >= width || y < 0 || y >= height) {
            x = y = 0;  // 超出范围，设置为零
        }
        break;

    case WrapMode::WrapClamp:
        // 对超出范围的坐标进行限制
        x = std::clamp(x, 0, width - 1);
        y = std::clamp(y, 0, height - 1);
        break;

    case WrapMode::WrapRepeat:
        // 对超出范围的坐标进行重复包裹
        x = x - std::floor(x / (float)width) * width;
        y = y - std::floor(y / (float)height) * height;
        break;

    case WrapMode::WrapPingPong:
        // 对超出范围的坐标进行反弹（ping-pong）
        if (x < 0) {
            x = -x;  // 反弹回去
        }
        else if (x >= width) {
            x = (width - 1) - (x - (width - 1));  // 反弹回去
        }

        if (y < 0) {
            y = -y;  // 反弹回去
        }
        else if (y >= height) {
            y = (height - 1) - (y - (height - 1));  // 反弹回去
        }
        break;
    }
}

// gpt o4 mini high 编写
// 使用 uv 坐标，自动msaa或者bilinear
Eigen::Vector4f Texture::Sample(float u, float v, FilterMode filterMode, WrapMode wrapMode) {
    // 转换为像素坐标
    int ix = (int)((1 - u) * (width - 1));
    int iy = (int)((1 - v) * (height - 1));
    LegalizationCoordinates(ix, iy, wrapMode);

    switch (filterMode) {
    case FilterMode::FilterNone:
        return GetPixel(ix, iy); // 没有过滤，直接返回像素值

    case FilterMode::FilterLinear: {
        // 双线性插值
        float fx = (1 - u) * (width - 1) - ix;  // 计算u方向的小数部分
        float fy = (1 - v) * (height - 1) - iy; // 计算v方向的小数部分

        // 获取四个邻域像素的颜色
        Eigen::Vector4f c00 = GetPixel(ix, iy);  // 左上角
        Eigen::Vector4f c10 = GetPixel(ix + 1, iy); // 右上角
        Eigen::Vector4f c01 = GetPixel(ix, iy + 1); // 左下角
        Eigen::Vector4f c11 = GetPixel(ix + 1, iy + 1); // 右下角

        // 双线性插值
        Eigen::Vector4f top = c00 * (1 - fx) + c10 * fx;  // 上边插值
        Eigen::Vector4f bottom = c01 * (1 - fx) + c11 * fx;  // 下边插值
        return top * (1 - fy) + bottom * fy;  // 最终插值
    }

    default:
        return GetPixel(ix, iy); // 如果没有匹配的模式，返回默认像素值
    }
}

Texture *Texture::Copy()
{
	Texture *tex = new Texture(width, height);
	this->Each([&tex](int x, int y, Eigen::Vector4f &value) 
	{
		tex->ReferDirect(x, y) = value;
	});
	return tex;
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
