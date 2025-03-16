#pragma once
#include "GraphicsType.h"

class SubpixelPostprocessingPass
{
public:
	virtual void init() = 0;
	virtual void fragment(SubpixelData &pixelData) = 0;
};

class PixelPostprocessingPass
{
public:
	virtual void init() = 0;
	virtual void fragment(PixelData &pixelData) = 0;
};
