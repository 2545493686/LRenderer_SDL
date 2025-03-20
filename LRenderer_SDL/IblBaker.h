#pragma once
class Cubemap;

class IblBaker
{
	IblBaker() = delete;

public:
	static Cubemap * BakeIrradiance(Cubemap *input);
};

