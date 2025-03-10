#pragma once

#include "Eigen/Dense"

// 透视矫正插值 
// https://zhuanlan.zhihu.com/p/144331875 孙小磊_计算机图形学六：透视矫正插值和图形渲染管线总结
// LANQ 25.2.25
class PerspectiveCorrectInterpolation
{
public:
	EIGEN_ALWAYS_INLINE static float InterpolationZ(Eigen::Vector3f barycentric, float z[3], bool isPerspective)
	{
		float a = barycentric.x();
		float b = barycentric.y();
		float c = barycentric.z();

		if (isPerspective)
		{
			return 1 / ((a / z[0]) + (b / z[1]) + (c / z[2]));
		}
		else
		{
			return a * z[0] + b * z[1] + c * z[2];
		}
	}

	// It = (a * Ia / Za + b * Ib / Zb + c * Ic / Zc) * zt
	template <typename T>
	EIGEN_ALWAYS_INLINE static T InterpolationVector(Eigen::Vector3f barycentric, float z[3], float zt, T vec[3], bool isPerspective)
	{
		float a = barycentric.x();
		float b = barycentric.y();
		float c = barycentric.z();

		if (isPerspective)
		{
			return (a * vec[0] / z[0] + b * vec[1] / z[1] + c * vec[2] / z[2]) * zt;
		}
		else
		{
			return a * vec[0] + b * vec[1] + c * vec[2];
		}
	}
};

using PCI = PerspectiveCorrectInterpolation;

