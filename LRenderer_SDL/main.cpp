#include "main.h"

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

    SDL_Window* window = SDL_CreateWindow("LRenderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    bool running = true;
    SDL_Event event;

    Framebuffer *framebuffer = new Framebuffer(WIDTH, HEIGHT);
    Buffer<float>* zBuffer = new Buffer<float>(WIDTH, HEIGHT);

    while (running) {
        // 事件处理
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        Draw(framebuffer, zBuffer);

        // 将帧缓冲绘制到窗口
        SDL_UpdateTexture(texture, nullptr, framebuffer->data(), WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        // 调试只绘制一帧
    }

    // 释放资源
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void Draw(Framebuffer *framebuffer, Buffer<float> *zBuffer)
{
    Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("assets\\cube.fbx", aiProcess_Triangulate | aiProcess_GenSmoothNormals);
    
    if (!scene)
    {
        spdlog::error(importer.GetErrorString());
        abort();
    }

	Transform *cameraTransform = new Transform();
	Camera *camera = new Camera(cameraTransform);
	camera->aspect = WIDTH / (float)HEIGHT;

    // 将 AIScene 转为 Mesh
	const aiMesh* aiMesh = scene->mMeshes[0];
	Mesh *mesh = MeshConverter::Covert(aiMesh);

	Transform *meshTransform = new Transform();
	meshTransform->position = Eigen::Vector3f(0, 0, 10);
    meshTransform->scale = Eigen::Vector3f(1, 1, 1);
    meshTransform->Rotate(45, 0, 0);

    UnlitShader *meshShader = new UnlitShader();

    framebuffer->clear(Color::Black); // 黑色背景
    // zBuffer 填充正无穷
	zBuffer->clear(std::numeric_limits<float>::infinity()); // 最大值

    Graphics::SetFramebuffer(framebuffer);
    Graphics::SetZBuffer(zBuffer);
    
    Graphics::DrawMesh(mesh, meshTransform->GetModelMatrix(), meshShader);
}
