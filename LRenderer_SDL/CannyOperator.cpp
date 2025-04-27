#define _USE_MATH_DEFINES

#include "CannyOperator.h"

#include <stack>
#include <cmath>

void CannyOperator::Invoke(Texture *tex)
{
	auto dx = this->dx;
	auto dy = this->dy;

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
	Eigen::MatrixXi highThreshold(tex->width, tex->height);
	Eigen::MatrixXi lowThreshold(tex->width, tex->height);

	float maxVal = tex->ReferDirect(0, 0).x();
	tex->Each([&maxVal](int x, int y, Eigen::Vector4f &value) {
		maxVal = std::max(maxVal, value.x());
	});

	tex->Each([&highThreshold, &lowThreshold, &maxVal](int x, int y, Eigen::Vector4f &value)
	{
		float v = value.x();
		highThreshold(x, y) = v > (maxVal * 0.3f);
		lowThreshold(x, y) = v > (maxVal * 0.1f);
	});
	
	// 连接边
	auto visitStack = std::stack<std::pair<int, int>>();

	tex->Each([&highThreshold, &lowThreshold, &visitStack, &tex, &dx, &dy](int texX, int texY)
	{
		if (highThreshold(texX, texY))
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
					
					if (lowThreshold(sx, sy) && !highThreshold(sx, sy))
					{
						highThreshold(sx, sy) = 1;
						visitStack.push(std::make_pair(sx, sy));
					}
				}
			}
		}
	});

	tex->Each([&highThreshold, &lowThreshold](int x, int y, Eigen::Vector4f &value)
	{
		value = Eigen::Vector4f::Ones() * highThreshold(x, y);
	});

	delete temp;
}
