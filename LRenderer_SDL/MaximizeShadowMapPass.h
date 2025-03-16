#pragma once
#include "PostprocessingPass.h"

class MaximizeShadowMapPass : public PixelPostprocessingPass
{
public:
	// Inherited via PostprocessingPass
	void init() override;
	void fragment(PixelData &pixelData) override;
};

