#pragma once
#include <Eigen/Dense>
#include "TextureOperator.h"

class CannyOperator : public TextureOperator
{
public:
	// Inherited via TextureOperator
	void Invoke(Texture *tex) override;

private:
	std::pair<Eigen::MatrixXf, Eigen::MatrixXf> GetGaussianDerivativeKernel(float sigma);
};

