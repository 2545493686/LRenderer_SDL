#pragma once
#include <vector>

class Cubemap;

class MipmapBaker
{
	MipmapBaker() = delete;

public:
	static std::vector<Cubemap *> BakeMipmap(Cubemap *basis);
};

