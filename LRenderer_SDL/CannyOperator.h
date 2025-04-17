#pragma once
#include "TextureOperator.h"

class CannyOperator : public TextureOperator
{
public:
	// Inherited via TextureOperator
	void Invoke(Texture *tex) override;
};

