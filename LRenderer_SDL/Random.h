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

class Random
{
public:
	EIGEN_ALWAYS_INLINE static SquareRandomProvider InSquare(float diameter)
	{
		return SquareRandomProvider(diameter);
	}
};