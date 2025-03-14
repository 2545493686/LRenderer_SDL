#pragma once
#include "PostprocessingPass.h"
#include "Framebuffer.h"
#include "Camera.h"

class InitShadowMapPass : public PostprocessingPass
{
public:
	Camera *camera;
	Framebuffer *shadowMap;

	virtual void init() override;
	virtual void fragment(SubpixelData &pixelData) override;

private:
	Eigen::Matrix4f worldToClipMatrix;
};

