#include "MaximizeShadowMapPass.h"

#include <algorithm>

void MaximizeShadowMapPass::init()
{
}

void MaximizeShadowMapPass::fragment(PixelData &pixelData)
{
	float maxDepth = pixelData.subpixels[0].z;

	for (size_t i = 1; i < pixelData.subpixels.size(); i++)
	{
		maxDepth = std::max(maxDepth, pixelData.subpixels[i].z);
	}

	pixelData.subpixels[0].z = maxDepth;
}
