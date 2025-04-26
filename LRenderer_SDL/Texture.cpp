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
// 使用 uv 坐标，自动msaa或者bilinear
Eigen::Vector4f Texture::Sample(float x, float y, FilterMode filterMode, WrapMode wrapMode) {
    // 纹理坐标包裹处理
    switch (wrapMode) {
    case WrapMode::WrapNone:
        if (x < 0.0f || x >= 1.0f || y < 0.0f || y >= 1.0f)
            return Eigen::Vector4f::Zero(); // 如果坐标超出范围，返回零
        break;

    case WrapMode::WrapClamp:
        // 对超出范围的坐标进行限制
        x = std::clamp(x, 0.0f, 1.0f);
        y = std::clamp(y, 0.0f, 1.0f);
        break;

    case WrapMode::WrapRepeat:
        // 对超出范围的坐标进行重复包裹
        x = x - std::floor(x);
        y = y - std::floor(y);
        break;

    case WrapMode::WrapPingPong:
        // 对超出范围的坐标进行反弹（ping-pong）
        if (x < 0.0f) {
            x = -x;  // 反弹回去
        }
        else if (x >= 1.0f) {
            x = 2.0f - x;  // 反弹回去
        }

        if (y < 0.0f) {
            y = -y;  // 反弹回去
        }
        else if (y >= 1.0f) {
            y = 2.0f - y;  // 反弹回去
        }
        break;
    }

    // 转换为像素坐标
    int ix = (int)((1 - x) * (width - 1));
    int iy = (int)((1 - y) * (height - 1));

    switch (filterMode) {
    case FilterMode::FilterNone:
        return GetPixel(ix, iy); // 没有过滤，直接返回像素值

    case FilterMode::FilterLinear: {
        // 双线性插值
        float fx = (1 - x) * (width - 1) - ix;  // 计算u方向的小数部分
        float fy = (1 - y) * (height - 1) - iy; // 计算v方向的小数部分

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
