#pragma once
#include "Texture.h"

class TextureOperator
{
public:
	virtual void Invoke(Texture *tex) = 0;
};

