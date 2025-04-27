#pragma once
#include <Eigen/Dense>
#include "TextureOperator.h"

class CannyOperator : public TextureOperator
{
public:
	// input
	Texture *dx;
	Texture *dy;

	// Inherited via TextureOperator
	void Invoke(Texture *tex) override;
};

