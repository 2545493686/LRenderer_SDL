#pragma once
#include <corecrt_math_defines.h>
#include <random>

#include "Eigen/Dense"

template <typename T>
class RandomProvider
{
public:
	virtual T Pop() = 0;

protected:
	std::mt19937 engine{std::random_device{}()};
};

class SquareRandomProvider : public RandomProvider<Eigen::Vector2f>
{
public:
	SquareRandomProvider(float diameter) : diameter(diameter) 
	{
		dist = std::uniform_real_distribution<float>(-diameter / 2, diameter / 2);
	}

	Eigen::Vector2f Pop() override
	{
		return Eigen::Vector2f(dist(this->engine), dist(this->engine));
	}

private:
	std::uniform_real_distribution<float> dist;
	float diameter;
};

class RangeRandomProvider : public RandomProvider<float>
{
public:
	RangeRandomProvider(float minIncludedValue, float maxIncludedValue)
	{
		float max_adjusted = std::nextafter(maxIncludedValue, std::numeric_limits<float>::max());
		dist = std::uniform_real_distribution<float>(-minIncludedValue, max_adjusted);
		min = minIncludedValue;
		max = maxIncludedValue;
	}
		
	float Pop() override
	{
		return std::clamp(dist(this->engine), min, max);
	}

private:
	std::uniform_real_distribution<float> dist;
	float min;
	float max;
};

class CircleRandomProvider : public RandomProvider<Eigen::Vector2f>
{
public:
	CircleRandomProvider(float radius) : radius(radius)
	{
		dist = std::uniform_real_distribution<float>(0, radius);
	}

	Eigen::Vector2f Pop() override
	{
		float r = radius * std::sqrtf(dist(this->engine));
		float theta = dist(this->engine) * 2 * M_PI;
        return Eigen::Vector2f(r * std::cosf(theta), r * std::sinf(theta));
	}

private:
	std::uniform_real_distribution<float> dist;
	float radius;
};

class Random
{
public:
	EIGEN_ALWAYS_INLINE static SquareRandomProvider InSquare(float diameter)
	{
		return SquareRandomProvider(diameter);
	}

	EIGEN_ALWAYS_INLINE static RangeRandomProvider InRange(float minIncludedValue, float maxIncludedValue)
	{
		return RangeRandomProvider(minIncludedValue, maxIncludedValue);
	}

	EIGEN_ALWAYS_INLINE static CircleRandomProvider InCircle(float radius)
	{
		return CircleRandomProvider(radius);
	}
};