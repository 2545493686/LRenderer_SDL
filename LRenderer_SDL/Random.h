#pragma once
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
	}

	float Pop() override
	{
		return dist(this->engine);
	}

private:
	std::uniform_real_distribution<float> dist;
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
};