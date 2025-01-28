#include <SDL.h>
#include <SDL_main.h>
#include <vector>
#include <iostream>
#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>  // For file sink

#include "main.h"
#include "Framebuffer.h"
#include "Color.h"
#include "MeshConverter.h"


// 窗口宽高
const int WIDTH = 800;
const int HEIGHT = 600;

int main(int argc, char* argv[]) {

    // 日志系统
    auto file_logger = spdlog::basic_logger_mt("file_logger", "logs/main_log.txt");
    spdlog::set_default_logger(file_logger);
    spdlog::flush_on(spdlog::level::err);

    // 告诉 SDL 我们将自己处理 main 函数
    SDL_SetMainReady();

    // 初始化 SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Soft Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    bool running = true;
    SDL_Event event;

    Framebuffer framebuffer(WIDTH, HEIGHT);

    while (running) {
        // 事件处理
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // 清屏
        framebuffer.clear(0xFF000000); // 黑色背景

        DrawFramebuffer(framebuffer);

        // 将帧缓冲绘制到窗口
        SDL_UpdateTexture(texture, nullptr, framebuffer.data(), WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    // 释放资源
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void DrawFramebuffer(Framebuffer &framebuffer)
{
    // 绘制简单像素点
    for (int x = 100; x < 600; ++x) {
        framebuffer.putPixel(x, 150, Color::Red); // 红色直线
    }

    framebuffer.drawLine(0, 0, 100, 200, Color::Green);
    framebuffer.drawLine(0, 0, framebuffer.getWidth() / 2, 
        framebuffer.getHeight() / 2, Color::Yellow);

    // 假设 pResourceData 是你已经加载的模型数据
    Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("assets\\houtou.fbx", aiProcess_Triangulate | aiProcess_GenSmoothNormals);
    
    if (!scene)
    {
        spdlog::error(importer.GetErrorString());
        abort();
    }

    // 将 AIScene 转为 Mesh
	const aiMesh* aiMesh = scene->mMeshes[0];
	Mesh mesh = MeshConverter::Covert(aiMesh);
    
    // 屏幕中心点
	Eigen::Vector3f center(framebuffer.getWidth() / 2, 0, framebuffer.getHeight() / 2);
    
    Eigen::Vector3f rightDown(framebuffer.getWidth(), 0, framebuffer.getHeight());

    // 打印mesh顶点数量
    for (size_t i = 1; i < mesh.edgesCount; i++)
    {
		Eigen::Vector3f p0 = rightDown - mesh.vertices[mesh.edges[i - 1]] * 250 - center;
        Eigen::Vector3f p1 = rightDown - mesh.vertices[mesh.edges[i]] * 250 - center;

		framebuffer.drawLine(p0.x(), p0.z(), p1.x(), p1.z(), Color::White);
    }
}
