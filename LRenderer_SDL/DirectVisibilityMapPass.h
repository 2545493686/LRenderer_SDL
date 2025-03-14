#pragma once
#include "PostprocessingPass.h"

template<typename T>
class Buffer;
class Camera;
class Framebuffer;

class DirectVisibilityMapPass : public PostprocessingPass
{
public:
	Framebuffer *shadowMapBuffer;
	Buffer<float> *directVisibilityMap;
	Camera *shadowCamera;

	// Inherited via PostprocessingPass
	void init() override;
	void fragment(SubpixelData &pixelData) override;

private:
	Eigen::Matrix4f worldToClipMatrix;
};

