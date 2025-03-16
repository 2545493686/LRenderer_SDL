#pragma once
#include "PostprocessingPass.h"

template<typename T>
class Buffer;
class Camera;
class Framebuffer;
class Light;
enum class LightType;

class DirectVisibilityMapPass : public SubpixelPostprocessingPass
{
public:
	Framebuffer *shadowMapBuffer;
	Buffer<float> *directVisibilityMap;
	Camera *shadowCamera;
	Light *light;

	int rayCount = 7;
	int raySampleCount = 8;

	// Inherited via PostprocessingPass
	void init() override;
	bool TestPointVisibility(const Eigen::Vector4f &worldPos);
	bool TestRayVisibility(const Eigen::Vector4f &worldPos, const Eigen::Vector4f &ray, float basicBias);
	void fragment(SubpixelData &pixelData) override;

private:
	LightType lightType;
	float floatError;
	float maxBias;
	float minBias;
	int twoPowSampleCount;

	float minRadius;
	float maxRadius;

	Eigen::Matrix4f worldToClipMatrix;
	Eigen::Matrix4f viewToWorld;
};

