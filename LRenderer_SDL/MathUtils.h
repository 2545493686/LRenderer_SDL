#pragma once

#include <algorithm>
#include "cmath"
#include "Eigen/Dense"

EIGEN_ALWAYS_INLINE
static Eigen::Vector4f vec4(float v)
{
	return Eigen::Vector4f::Ones() * v;
}

EIGEN_ALWAYS_INLINE
static Eigen::Vector3f vec3(float v)
{
	return Eigen::Vector3f::Ones() * v;
}

EIGEN_ALWAYS_INLINE
static Eigen::Vector4d vec4d(double v)
{
	return Eigen::Vector4d::Ones() * v;
}

class MathUtils
{
public:
	EIGEN_ALWAYS_INLINE static Eigen::Vector4f Reflect(Eigen::Vector4f viewDirection, Eigen::Vector4f worldNormal)
	{
		return 2 * viewDirection.dot(worldNormal) * worldNormal - viewDirection;
	}

	EIGEN_ALWAYS_INLINE static Eigen::Vector4f Pow(Eigen::Vector4f v, float p)
	{
        return v.array().pow(p);
	}

	EIGEN_ALWAYS_INLINE static Eigen::Vector4f Lerp(Eigen::Vector4f p1, Eigen::Vector4f p2, float v)
	{
		v = std::clamp(v, 0.0f, 1.0f);

		return p1 * (1 - v) + p2 * v;
	}

	EIGEN_ALWAYS_INLINE static float Cross(Eigen::Vector2f p1, Eigen::Vector2f p2)
	{
		return p1.x() * p2.y() - p1.y() * p2.x();
	}

	EIGEN_ALWAYS_INLINE static bool InTriangle(
			Eigen::Vector2f point, Eigen::Vector2f trianglePos0, Eigen::Vector2f trianglePos1, Eigen::Vector2f trianglePos2)
	{
		int c1 = Cross(trianglePos1 - trianglePos0, point - trianglePos0);
		int c2 = Cross(trianglePos2 - trianglePos1, point - trianglePos1);
		int c3 = Cross(trianglePos0 - trianglePos2, point - trianglePos2);

		return (c1 >= 0 && c2 >= 0 && c3 >= 0) || (c1 <= 0 && c2 <= 0 && c3 <= 0);
	}

	// 重心坐标 
	// 源自 Games101 作业2  25.2.5
	EIGEN_ALWAYS_INLINE static Eigen::Vector3f Barycentric(
		Eigen::Vector2f point, Eigen::Vector2f trianglePos0, Eigen::Vector2f trianglePos1, Eigen::Vector2f trianglePos2)
	{
		float x = point.x();
		float y = point.y();
		Eigen::Vector2f v[3] = { trianglePos0, trianglePos1, trianglePos2 };

		float c1 = (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y + v[1].x() * v[2].y() - v[2].x() * v[1].y()) / (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() + v[1].x() * v[2].y() - v[2].x() * v[1].y());
		float c2 = (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y + v[2].x() * v[0].y() - v[0].x() * v[2].y()) / (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() + v[2].x() * v[0].y() - v[0].x() * v[2].y());
		float c3 = (x * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * y + v[0].x() * v[1].y() - v[1].x() * v[0].y()) / (v[2].x() * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * v[2].y() + v[0].x() * v[1].y() - v[1].x() * v[0].y());

		return Eigen::Vector3f(c1, c2, c3);
	}

	template<typename T> EIGEN_ALWAYS_INLINE static 
	T Clamp(T v, T min, T max)
	{
		return std::max(min, std::min(v, max));
	}

	EIGEN_ALWAYS_INLINE static
	void RemoveNan(Eigen::Vector4d &v, float t)
	{
		if (std::isnan(v.x()))
		{
			v.x() = t;
		}
		if (std::isnan(v.y()))
		{
			v.y() = t;
		}
		if (std::isnan(v.z()))
		{
			v.z() = t;
		}
		if (std::isnan(v.w()))
		{
			v.w() = t;
		}
	}

	EIGEN_ALWAYS_INLINE static
	void RemoveNan(Eigen::Vector4f &v, float t)
	{
		if (std::isnan(v.x()))
		{
			v.x() = t;
		}
		if (std::isnan(v.y()))
		{
			v.y() = t;
		}
		if (std::isnan(v.z()))
		{
			v.z() = t;
		}
		if (std::isnan(v.w()))
		{
			v.w() = t;
		}
	}

	EIGEN_ALWAYS_INLINE static
	bool IsNan(const Eigen::Vector4d &v)
	{
		for (size_t i = 0; i < 4; i++)
		{
			if (std::isnan(v[i]))
			{
				return true;
			}
		}
		return false;
	}

	EIGEN_ALWAYS_INLINE static
	bool IsNan(const Eigen::Vector4f &v)
	{
		for (size_t i = 0; i < 4; i++)
		{
			if (std::isnan(v[i]))
			{
				return true;
			}
		}
		return false;
	}
	EIGEN_ALWAYS_INLINE static
	void ClampVector4(Eigen::Vector4f &v, float min, float max)
	{
		RemoveNan(v, 0);

		v.x() = std::clamp(v.x(), min, max);
        v.y() = std::clamp(v.y(), min, max);
        v.z() = std::clamp(v.z(), min, max);
        v.w() = std::clamp(v.w(), min, max);
	}

	EIGEN_ALWAYS_INLINE static
	void ClampVector4(Eigen::Vector4d &v, double min, double max)
	{
		RemoveNan(v, 0);

		v.x() = std::clamp(v.x(), min, max);
		v.y() = std::clamp(v.y(), min, max);
		v.z() = std::clamp(v.z(), min, max);
		v.w() = std::clamp(v.w(), min, max);
	}
};

