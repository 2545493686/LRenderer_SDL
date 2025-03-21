#pragma once
#include <SDL2/SDL_image.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>  // For file sink

#include "Texture.h"

class TextureLoader
{
public:
	TextureLoader() = delete;

    static Texture* LoadPNG(const char* path)
    {
		return LoadPNG_BySDL(path);
    }

private:
	static Texture* LoadPNG_BySDL(const char* path)
	{
		SDL_Surface* surface = IMG_Load(path);
		if (!surface) {
			spdlog::error(IMG_GetError());
			abort();
		}

		SDL_Surface* convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
		Texture* tex = new Texture(convertedSurface->w, convertedSurface->h);

		// 将图像数据复制到纹理
		for (int y = 0; y < convertedSurface->h; y++) {
			for (int x = 0; x < convertedSurface->w; x++) {
				Uint32* row = (Uint32*)((Uint8*)convertedSurface->pixels + y * convertedSurface->pitch);
				Uint32 pixel = row[x];
				Uint8 r, g, b, a;
				SDL_GetRGBA(pixel, convertedSurface->format, &r, &g, &b, &a);

				Eigen::Vector4f color;
				color.x() = std::pow(r / 255.0f, 2.2f);
				color.y() = std::pow(g / 255.0f, 2.2f);
                color.z() = std::pow(b / 255.0f, 2.2f);
                color.w() = a / 255.0f;

				tex->PutPixel(x, y, color);
			}
		}

		SDL_FreeSurface(convertedSurface);

		return tex;
	}

};

