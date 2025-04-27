#include "HoughOperator.h"
#include "unsupported/Eigen/CXX11/Tensor"

#include <iostream>
#include <algorithm>
#include <vector>

// deepseek 编写
std::vector<Eigen::Vector2i> HoughOperator::GetLinePoints(Eigen::Vector2f begin, Eigen::Vector2f dir, float maxR) {
    std::vector<Eigen::Vector2i> points;

    // 归一化方向向量
    Eigen::Vector2f direction = dir.normalized();

    // 计算正反方向终点
    Eigen::Vector2f endPos = begin + direction * maxR;
    Eigen::Vector2f endNeg = begin - direction * maxR;

    // 生成正向延伸的点（包含起点）
    DDALine(begin, endPos, points, true);

    // 生成反向延伸的点（排除起点避免重复）
    DDALine(begin, endNeg, points, false);

    return points;
}

// deepseek 编写
void HoughOperator::DDALine(Eigen::Vector2f start, Eigen::Vector2f end,
    std::vector<Eigen::Vector2i> &points, bool includeStart) {
    // 坐标取整规则（四舍五入）
    int x0 = static_cast<int>(std::round(start.x()));
    int y0 = static_cast<int>(std::round(start.y()));
    int x1 = static_cast<int>(std::round(end.x()));
    int y1 = static_cast<int>(std::round(end.y()));

    // 计算步长和步数
    float dx = x1 - x0;
    float dy = y1 - y0;
    float steps = std::max(std::abs(dx), std::abs(dy));

    // 处理零长度线段
    if (steps == 0) {
        if (includeStart) points.emplace_back(x0, y0);
        return;
    }

    // 计算增量
    float xIncrement = dx / steps;
    float yIncrement = dy / steps;

    // 生成点集
    float x = x0;
    float y = y0;
    for (int i = 0; i <= steps; ++i) {
        // 排除起点（仅用于反向延伸）
        if (i == 0 && !includeStart) {
            x += xIncrement;
            y += yIncrement;
            continue;
        }

        // 四舍五入当前坐标
        Eigen::Vector2i point(static_cast<int>(std::round(x)),
            static_cast<int>(std::round(y)));

        // 避免重复添加相邻的相同点
        if (points.empty() || point != points.back()) {
            points.push_back(point);
        }

        x += xIncrement;
        y += yIncrement;
    }
}

void HoughOperator::DrawPoints(Texture *tex, std::vector<Eigen::Vector2i> points)
{
    for (auto &point : points)
    {
        tex->LegalizationCoordinates(point.x(), point.y());
        tex->ReferDirect(point.x(), point.y()) = Eigen::Vector4f(0, 0, 1, 1);
    }
}

void HoughOperator::DrawCircle(Texture *tex, int centerX, int centerY, int r)
{
    tex->Each([this, centerX, centerY, r](int x, int y, Eigen::Vector4f &value)
    {
        if (std::sqrtf(std::powf(x - centerX, 2) + std::powf(y - centerY, 2)) < r)
        {
            value = Eigen::Vector4f(0, 1, 0, 1);
        }
    });
}
void HoughOperator::Invoke(Texture *tex)
{
	int rRange = (int)std::sqrt(tex->width * tex->width + tex->height * tex->height) + 1;
	// 参数空间 x, y, r
	Eigen::Tensor<long long, 3> parameterSpace(tex->width, tex->height, rRange);
    parameterSpace.setZero();

    // LegalizationParameter 函数
    auto LegitimateParameter = [tex, rRange](int &x, int &y, int &r)
    {
        if ((x < 0) || (x > tex->width - 1))
        {
            return false;
        }
        
        if ((y < 0) || (y > tex->height - 1))
        {
            return false;
        }

        if ((r < 0) || (r > rRange - 1))
        {
            return false;
        }

        return true;
    };

    int flag = 0;
	tex->Each([this, &flag, &tex, rRange, &parameterSpace, &LegitimateParameter](int x, int y, Eigen::Vector4f &value)
	{

		if (value.x() > 0.00001f)
		{
            flag++;
            if (flag % 200)
            {
                //return;
            }
			auto vdx = dx->ReferDirect(x, y).x();
			auto vdy = dy->ReferDirect(x, y).y();

			auto gradient = Eigen::Vector2f(vdx, vdy).normalized();
			auto k = gradient.y() / gradient.x();

            Eigen::Vector2f lineBegin = Eigen::Vector2i(x, y).cast<float>();
            auto points = GetLinePoints(lineBegin, gradient, rRange);
            
            //DrawCircle(tex, lineBegin.x(), lineBegin.y(), 5);
            //DrawPoints(tex, points);

            for (auto point : points)
            {
                auto px = point.x();
                auto py = point.y();
                auto pp = Eigen::Vector2f(px, py);
                auto pr = static_cast<int>((pp - lineBegin).norm());

                if (LegitimateParameter(px, py, pr))
                {
                    parameterSpace(px, py, pr) += 7;
                }

                for (int i = -1; i <= 1; i++)
                {
                    for (int j = -1; j <= 1; j++)
                    {
                        auto rx = px + i;
                        auto ry = py + j;
                        auto rp = Eigen::Vector2f(rx, ry);
                        auto rr = static_cast<int>((rp - lineBegin).norm());

                        if (LegitimateParameter(rx, ry, rr))
                        {
                            parameterSpace(rx, ry, rr)++;
                        }
                    }
                }
            }
        }
	});

    int maxIndexX = 0;
    int maxIndexY = 0;
    int maxIndexR = 0;
    long long maxValue = 0;  
    for (int psz = 0; psz < parameterSpace.dimension(2); psz++)
    {
        for (int psy = 0; psy < parameterSpace.dimension(1); psy++)
        {
            for (int psx = 0; psx < parameterSpace.dimension(0); psx++)
            {  
                long long value = parameterSpace(psx, psy, psz);
                if (value > maxValue) 
                {
                    maxValue = value;
                    maxIndexX = psx;
                    maxIndexY = psy;
                    maxIndexR = psz;
                }
            }
        }
    }

    circlePad = new Texture(tex->width, tex->height);
    tex->Each([this](int x, int y, Eigen::Vector4f &value)
    {
        value = Eigen::Vector4f::Zero();
    });

    DrawCircle(circlePad, maxIndexX, maxIndexY, maxIndexR);
}