#pragma once

#include "Eigen/Dense"

// 透视矫正插值 
// https://zhuanlan.zhihu.com/p/144331875 孙小磊_计算机图形学六：透视矫正插值和图形渲染管线总结
// LANQ 25.2.25
class PerspectiveCorrectInterpolation
{
public:
	EIGEN_ALWAYS_INLINE static float InterpolationZ(Eigen::Vector3f barycentric, float z[3])
	{
		float a = barycentric.x();
		float b = barycentric.y();
		float c = barycentric.z();

		return 1 / ((a / z[0]) + (b / z[1]) + (c / z[2]));
	}

	// It = (a * Ia / Za + b * Ib / Zb + c * Ic / Zc) * zt
	EIGEN_ALWAYS_INLINE static Eigen::Vector4f InterpolationVector(Eigen::Vector3f barycentric, float z[3], float zt, Eigen::Vector4f vec[3])
	{
		float a = barycentric.x();
		float b = barycentric.y();
		float c = barycentric.z();

		return (a * vec[0] / z[0] + b * vec[1] / z[1] + c * vec[2] / z[2]) * zt;
	}
};

using PCI = PerspectiveCorrectInterpolation;

