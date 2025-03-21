#pragma once
class Cubemap;

class IblBaker
{
	IblBaker() = delete;

public:
	static Cubemap * BakeIrradiance(Cubemap *input);
	static Cubemap * BakeRadiance(Cubemap *input, float roughness, int inverseScale);
};

