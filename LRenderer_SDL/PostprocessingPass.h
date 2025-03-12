#pragma once
#include "GraphicsType.h"

class PostprocessingPass
{
public:
	virtual void init() = 0;
	virtual void fragment(SubpixelData &pixelData) = 0;
};

