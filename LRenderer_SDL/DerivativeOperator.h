#pragma once
#include "TextureOperator.h"
class DerivativeOperator : public TextureOperator
{
public:
	// output
	Texture *dx;
	Texture *dy;

	~DerivativeOperator()
	{
		if (dx)
		{
			delete dx;
		}
		if (dy)
		{
			delete dy;
		}
	}

	// Í¨¹ý TextureOperator ¼Ì³Ð
	void Invoke(Texture *tex) override;
};

