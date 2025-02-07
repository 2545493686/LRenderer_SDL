#include "TextureCreater.h"

Texture* TextureCreater::Create(SDL_Surface* surface)
{
	SDL_Surface* convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
	Texture *tex = new Texture(convertedSurface->w, convertedSurface->h);

	// 将图像数据复制到纹理
	for (int y = 0; y < convertedSurface->h; y++) {
		for (int x = 0; x < convertedSurface->w; x++) {
			Uint32* row = (Uint32*)((Uint8*)convertedSurface->pixels + y * convertedSurface->pitch);
			Uint32 pixel = row[x];
			Uint8 r, g, b, a;
			SDL_GetRGBA(pixel, convertedSurface->format, &r, &g, &b, &a);
			tex->PutPixel(x, y, Eigen::Vector4f(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f));
		}
	}

	SDL_FreeSurface(convertedSurface);

    return tex;
}
