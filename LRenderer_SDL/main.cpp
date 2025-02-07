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

    int imgFlags = IMG_INIT_PNG;
    int initializedFlags = IMG_Init(imgFlags);
    if ((initializedFlags & imgFlags) != imgFlags) {
        spdlog::error("SDL_image 初始化失败: %s", IMG_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("LRenderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    bool running = true;
    SDL_Event event;

    Framebuffer *framebuffer = new Framebuffer(WIDTH, HEIGHT);
    Buffer<float>* zBuffer = new Buffer<float>(WIDTH, HEIGHT);

    Scene *scene = CreateScene();

    while (running) {
        // 事件处理
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        Draw(framebuffer, zBuffer, scene);

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

Scene* CreateScene()
{
	Scene *scene = new Scene();

#pragma region 载入外部文件

    // cube
    Assimp::Importer importer;
    const aiScene *aiscene = importer.ReadFile("assets\\cube.fbx", aiProcess_Triangulate | aiProcess_GenSmoothNormals);
    if (!aiscene)
    {
        spdlog::error(importer.GetErrorString());
        abort();
    }
    const aiMesh *cubeAiMesh = aiscene->mMeshes[0];
    Mesh *cubeMesh = MeshConverter::Covert(cubeAiMesh);

    // texture
    SDL_Surface *uvTexSurface = IMG_Load("assets\\texture.png");
    if (!uvTexSurface) {
        spdlog::error(IMG_GetError());
        abort();
    }
	Texture* uvTex = TextureCreater::Create(uvTexSurface);


#pragma endregion

#pragma region 立方体
    GameObject *cube = new GameObject();

#pragma region Transform
    Transform* meshTransform = new Transform();
    meshTransform->position = Eigen::Vector3f(0, 0, 10);
    meshTransform->scale = Eigen::Vector3f(1, 1, 1);
    meshTransform->Rotate(30, 0, 0);
    cube->AddComponent(meshTransform);
#pragma endregion

#pragma region MeshRenderer
    MeshRenderer* meshRenderer = new MeshRenderer();

    meshRenderer->mesh = cubeMesh;

    UnlitShader* meshShader = new UnlitShader();
    meshShader->tex1 = uvTex;
	meshRenderer->shader = meshShader;
    cube->AddComponent(meshRenderer);
#pragma endregion

    scene->AddGameObject(cube);
#pragma endregion

    return scene;
}

void Draw(Framebuffer *framebuffer, Buffer<float> *zBuffer, Scene *scene)
{
	Transform *cameraTransform = new Transform();
	Camera *camera = new Camera(cameraTransform);
	camera->aspect = WIDTH / (float)HEIGHT;

    framebuffer->clear(Color::Black); // 黑色背景
    // zBuffer 填充正无穷
	zBuffer->clear(std::numeric_limits<float>::infinity()); // 最大值

    Graphics::SetFramebuffer(framebuffer);
    Graphics::SetZBuffer(zBuffer);
    
    for (auto gameObject : scene->GetGameObjects())
    {
        MeshRenderer* renderer = gameObject->GetComponent<MeshRenderer>();
        
        if (renderer == nullptr)
        {
            continue;
        }

        Transform *transform = gameObject->GetComponent<Transform>();

        Graphics::DrawMesh(renderer->mesh, transform->GetModelMatrix(), renderer->shader);
    }
}
