#pragma once

#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <limits>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>  // For file sink

#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>

#include "Framebuffer.h"
#include "Color.h"
#include "MeshLoader.h"
#include "UnlitShader.h"
#include "Graphics.h"
#include "Camera.h"
#include "Scene.h"
#include "MeshRenderer.h"
#include "Texture.h"
#include "TextureLoader.h"
#include "Cubemap.h"
#include "CubemapLoader.h"
#include "CubemapShader.h"

void LoadAssets();
void Draw(Framebuffer* framebuffer, Scene* scene);
Scene* CreateScene();
