#pragma once
#include "PostprocessingPass.h"

template<typename T>
class Buffer;
class Camera;
class Framebuffer;
class Light;

class DirectVisibilityMapPass : public PostprocessingPass
{
public:
	Framebuffer *shadowMapBuffer;
	Buffer<float> *directVisibilityMap;
	Camera *shadowCamera;
	Light *light;

	// Inherited via PostprocessingPass
	void init() override;
	void fragment(SubpixelData &pixelData) override;

private:
	float floatError;
	float maxBias;
	float minBias;
	Eigen::Matrix4f worldToClipMatrix;
};

