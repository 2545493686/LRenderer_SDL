#pragma once

#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>

#include <SDL2/SDL_image.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <limits>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>  // For file sink

#include "Framebuffer.h"
#include "Color.h"
#include "MeshConverter.h"
#include "UnlitShader.h"
#include "Graphics.h"
#include "Camera.h"
#include "Scene.h"
#include "MeshRenderer.h"
#include "Texture.h"
#include "TextureCreater.h"

void Draw(Framebuffer* framebuffer, Buffer<float>* zBuffer, Scene* scene);
Scene* CreateScene();
