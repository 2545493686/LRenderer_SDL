#include "main.h"

// 窗口宽高
const int WIDTH = 800;
const int HEIGHT = 600;

const Eigen::Vector2f subpixelBiasX4[4] = {
    Eigen::Vector2f(-0.25f, 0.25f),
    Eigen::Vector2f(0.25f, 0.25f),
    Eigen::Vector2f(-0.25f, -0.25f),
    Eigen::Vector2f(0.25f, -0.25f),
};

EIGEN_ALWAYS_INLINE Eigen::Vector2f GetSubpixelPointBias(int x, int y, int subpixelIndex)
{
#if MSAA_TYPE == MSAA_X4
    return subpixelBiasX4[subpixelIndex];
#endif // MSAA_TYPE == MSAA_X4

    throw std::exception("msaaCount is unknown type.");
}

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

    Framebuffer *framebuffer = InitFramebuffer();

    LoadAssets();
    Scene *scene = CreateScene();

    int frameCount = 0;

    while (running) {
        auto start = std::chrono::high_resolution_clock::now();

        // 事件处理
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        Draw(framebuffer, scene);

        // 将帧缓冲绘制到窗口
        SDL_UpdateTexture(texture, nullptr, framebuffer->colorBuffer.data(), WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
        
        frameCount++;
        auto end = std::chrono::high_resolution_clock::now();

        // 计算时间差并转换单位
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        Time::deltaTime = duration.count() / 1000.0f;
        Time::time += Time::deltaTime;


#if DEBUG_COUNT
        if (frameCount == DEBUG_COUNT)
        {
            while (1) {}
        }
#endif

        std::cout << "frame: " << frameCount << "\n";
        std::cout << "delta time: " << Time::deltaTime << "\n";
    }

    // 释放资源
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

Mesh* cubeMesh;
Mesh* sphereMesh;
Texture* uvTex;
Cubemap* skybox;

void LoadAssets()
{
    cubeMesh = MeshLoader::Load("assets\\cube.fbx");
    sphereMesh = MeshLoader::Load("assets\\sphere.fbx");
    uvTex = TextureLoader::LoadPNG("assets\\texture.png");
    skybox = CubemapLoader::LoadVerticalEXR("assets\\skybox_default.exr");
}

Scene* CreateScene()
{
	Scene *scene = new Scene();

#pragma region 立方体
    GameObject *cube = new GameObject();

#pragma region Transform
    Transform* meshTransform = new Transform();
    meshTransform->position = Eigen::Vector3f(2, 0, -10);
    meshTransform->scale = Eigen::Vector3f(1, 1, 1);
    meshTransform->Rotate(10, 0, 0);
    cube->AddComponent(meshTransform);
#pragma endregion

#pragma region MeshRenderer
    MeshRenderer* meshRenderer = new MeshRenderer(meshTransform);

    meshRenderer->mesh = cubeMesh;

    // 默认着色器
    BlinnPhongShader* meshShader = new BlinnPhongShader();
    meshShader->tex1 = uvTex;

	meshRenderer->shader = meshShader;
    cube->AddComponent(meshRenderer);
#pragma endregion

    scene->AddGameObject(cube);
#pragma endregion


#pragma region 球体
    GameObject* sphere = new GameObject();

#pragma region Transform
    Transform* sphereTransform = new Transform();
    sphereTransform->position = Eigen::Vector3f(-2, 0, -10);
    sphereTransform->scale = Eigen::Vector3f(1, 1, 1);
    sphere->AddComponent(sphereTransform);
#pragma endregion

#pragma region MeshRenderer
    MeshRenderer* sphereRenderer = new MeshRenderer(sphereTransform);
    sphereRenderer->mesh = sphereMesh;

    // 默认着色器
    BlinnPhongShader* sphereShader = new BlinnPhongShader();
    sphereShader->tex1 = uvTex;

    sphereRenderer->shader = sphereShader;
    sphere->AddComponent(sphereRenderer);
#pragma endregion

    scene->AddGameObject(sphere);
#pragma endregion

    return scene;
}

// 初始化空间，定义每个像素4个采样点
Framebuffer* InitFramebuffer()
{
    Framebuffer *framebuffer = new Framebuffer(WIDTH, HEIGHT);

    for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
    {
        for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
        {
            auto& pixelData = framebuffer->pixelBuffer.referPixel(x, y);
            pixelData.subpixels.resize(MSAA_TYPE);
        }
    }

    return framebuffer;
}


void ClearFramebuffer(Framebuffer* framebuffer)
{
    static auto provider = Random::InSquare(0.5);

#if SUBPIXEL_BIAS
    const Eigen::Vector2f bias = provider.Pop();
#else
    const Eigen::Vector2f bias = Eigen::Vector2f::Zero();
#endif // SUBPIXEL_BIAS

    // 必要的初始化
    for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
    {
        for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
        {
            auto& pixelData = framebuffer->pixelBuffer.referPixel(x, y);

            for (size_t subpixelIndex = 0; subpixelIndex < pixelData.subpixels.size(); subpixelIndex++)
            {
                auto& subpixel = pixelData.subpixels[subpixelIndex];

                subpixel.screenPosition = Eigen::Vector2f(x + 0.5f, y + 0.5f);
                subpixel.screenPosition += GetSubpixelPointBias(x, y, subpixelIndex) + bias;
                subpixel.color = Color::MakeVector(Color::Black);
                subpixel.shader = nullptr;
                subpixel.z = std::numeric_limits<float>::max();
            }
        }
    }
}

void Draw(Framebuffer *framebuffer, Scene *scene)
{
	static Transform *cameraTransform = new Transform();
    //static OrthographicCamera* camera = new OrthographicCamera(cameraTransform, WIDTH / (float)HEIGHT);
    static PerspectiveCamera* camera = new PerspectiveCamera(cameraTransform, WIDTH / (float)HEIGHT);
    
#if CAMERA_MOVE
    float moveSpeed = 1.0f;
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
    // 检测特定按键是否被按下
    if (keyboard_state[SDL_SCANCODE_UP]) {
        camera->transform->Rotate(moveSpeed, 0, 0);
    }
    if (keyboard_state[SDL_SCANCODE_DOWN]) {
        camera->transform->Rotate(-moveSpeed, 0, 0);
    }
    if (keyboard_state[SDL_SCANCODE_LEFT]) {
        camera->transform->Rotate(0, moveSpeed, 0);
    }
    if (keyboard_state[SDL_SCANCODE_RIGHT]) {
        camera->transform->Rotate(0, -moveSpeed, 0);
    }

#endif // CAMERA_MOVE


    Graphics::SetCamera(camera);

    ClearFramebuffer(framebuffer);
    Graphics::SetFramebuffer(framebuffer);

    static DirectionalLight* light = new DirectionalLight();
    light->direction = Eigen::Vector4f(-0.3, -0.8, 0.5, 0);

    Graphics::SetLight(light);

    auto sbb = scene->GetSphereBoudingBox();

    Eigen::Vector3f lightForward = light->direction.head<3>().normalized();


    //auto target = scene->gameObjects.data[0]->GetComponent<Transform>()->position;
    //auto dir = target - camera->transform->position;

    //camera->transform->position = sbb.center - lightForward * (sbb.radius + camera->zNear);
    //camera->transform->rotation = Eigen::Quaternionf::FromTwoVectors(
    //    -Eigen::Vector3f::UnitZ(),
    //    lightForward
    //);
    //camera->size = sbb.radius * 2;

    auto cube = scene->gameObjects.data[0]->GetComponent<Transform>();
    cube->Rotate(10, 10, 0);

    Eigen::Vector3f tp = sbb.center - lightForward * (sbb.radius + camera->zNear + 1);
    Graphics::DrawSphere(tp, 0.3f, Color::MakeVector(Color::Green));

    Eigen::Vector4f ambientLightColor = Color::MakeVector(Color::White) * 0.25f;
    Graphics::SetAmbientLightColor(ambientLightColor);

    PreDrawAllMeshes(framebuffer, scene);

    //static Transform* shadowCameraTransform = new Transform();

    static Camera* shadowCamera = new PerspectiveCamera(cameraTransform, WIDTH / (float)HEIGHT);

    Graphics::DrawFullScreen();

    static SkyboxShader* skyboxShader = new SkyboxShader();
    skyboxShader->tex1 = skybox;
    Graphics::DrawSkybox(skyboxShader);

#if !CAMERA_MOVE
    Graphics::DrawTAA();
#endif // !CAMERA_MOVE

    Graphics::DrawSphere(sbb.center, sbb.radius, Color::MakeVector(Color::Red));

    Graphics::MergeSubpixels();

    framebuffer->colorBuffer.drawImage(skybox->data[2], skybox->size, skybox->size, 128, 128);
    framebuffer->colorBuffer.drawLine(Eigen::Vector2f(0, 0), 
        Eigen::Vector2f(40, 30), Color::Yellow);
}

void PreDrawAllMeshes(Framebuffer *framebuffer, Scene *scene, Shader* shader)
{
    for (size_t i = 0; i < scene->gameObjects.data.size(); i++)
    {
        if (!scene->gameObjects.valid[i])
        {
            continue;
        }

        auto gameObject = scene->gameObjects.data[i];

        MeshRenderer* renderer = gameObject->GetComponent<MeshRenderer>();

        if (renderer == nullptr)
        {
            continue;
        }

        Transform* transform = gameObject->GetComponent<Transform>();

        Graphics::PreDrawMesh(renderer->mesh, transform->GetModelMatrix(), shader ? shader : renderer->shader);
    }
}