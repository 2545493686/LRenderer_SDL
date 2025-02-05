#pragma once

#include <SDL.h>
#include <SDL_main.h>
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


void Draw(Framebuffer *framebuffer, Buffer<float>* zBuffer);
