#pragma once
#include "TextureOperator.h"

class GrayscaleOperator : public TextureOperator
{
public:
	// Í¨¹ý TextureOperator ¼Ì³Ð
	void Invoke(Texture *tex) override;
};

