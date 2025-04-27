#pragma once
#include "TextureOperator.h"

class SobelOperator : TextureOperator
{
public:
	// output
	Texture *dx;
	Texture *dy;
	
	~SobelOperator()
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

