#pragma once

#include "Texture.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "spdlog/spdlog.h"

class TextureCreater
{
public:
	static Texture* Create(SDL_Surface* surface);
};

