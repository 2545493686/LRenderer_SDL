#include "main.h"

#include <format>
#include <string>

#include "SDL2/SDL_image.h"
#include "InitShadowMapPass.h"
#include "MaximizeShadowMapPass.h"
#include "IblBaker.h"
#include "CookTorranceShader.h"
#include "MipmapBaker.h"

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

int main(int argc, char *argv[]) {

#if BOOT_MODE == BOOT_GAME
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

	SDL_Window *window = SDL_CreateWindow("LRenderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

	bool running = true;
	SDL_Event event;

	DrawContext context;

	context.framebuffer = InitFramebuffer(WIDTH, HEIGHT);
	context.taaBuffer = InitTAABuffer(WIDTH, HEIGHT);
	context.shadowMap = new Framebuffer(4096, 4096);

	context.directVisibilityMap.resize(1);
	context.directVisibilityMap[0] = new Buffer<float>(WIDTH, HEIGHT);
	context.directVisibilityMap[0]->clear(1);

	LoadAssets();

	context.scene = CreateScene();

	int frameCount = 0;

	while (running) {
		auto start = std::chrono::high_resolution_clock::now();

		// 事件处理
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
		}

		Draw(context);

		// 将帧缓冲绘制到窗口
		SDL_UpdateTexture(texture, nullptr, context.framebuffer->colorBuffer.data(), WIDTH * sizeof(uint32_t));
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
#endif // BOOT_MODE == BOOT_GAME

#if BOOT_MODE == BOOT_IRRADIANCE_BAKER
	std::cout << "Start Irradiance Baker.\n";

	auto skybox = CubemapLoader::LoadVerticalEXR("assets\\skybox_default.exr");

	std::cout << "Bake Irradiance...\n";
	auto irradiance = IblBaker::BakeIrradiance(skybox);

	std::cout << "Save...\n";
	CubemapLoader::SaveVerticalEXR("assets\\skybox_default_irradiance_128K.exr", irradiance);

	std::cout << "Success.\n";
#endif // BOOT_MODE == BOOT_IRRADIANCE_BAKER

#if BOOT_MODE == BOOT_MIPMAPS_BAKER
	std::cout << "Start Mipmaps Baker.\n";

	auto skybox = CubemapLoader::LoadVerticalEXR("assets\\skybox_default.exr");

	std::cout << "Bake Mipmaps...\n";
	auto mipmaps = MipmapBaker::BakeMipmap(skybox);

	for (size_t i = 1; i < mipmaps.size(); i++)
	{
		std::cout << std::format("Save Mipmap {}\n", i);

		auto mipmap = mipmaps[i];
        CubemapLoader::SaveVerticalEXR(std::format("assets\\skybox_default_mipmap_{}.exr", i).c_str(), mipmap);
	}

	std::cout << "Success.\n";

#endif // BOOT_MODE == BOOT_MIPMAPS_BAKER

#if BOOT_MODE == BOOT_RADIANCE_BAKER

	std::cout << "Start Radiance Baker.\n";

	auto skybox = CubemapLoader::LoadVerticalEXR("assets\\skybox_default.exr");

	
	for (size_t i = 1; i <= 10; i++)
	{
		std::cout << std::format("Bake Radiances Map {}...\n", i);
		auto radianceMap = IblBaker::BakeRadiance(skybox, std::min(0.1f * i, 1.0f), pow(2, i));
		
		std::cout << std::format("Save Radiance Map {}\n", i);
		CubemapLoader::SaveVerticalEXR(std::format("assets\\skybox_default_radiance_{}.exr", i).c_str(), radianceMap);
	}

	std::cout << "Success.\n";

#endif // BOOT_MODE == BOOT_RADIANCE_BAKER


	return 0;
}

Mesh *cubeMesh;
Mesh *sphereMesh;
Mesh *planeMesh;
Texture *uvTex;
Cubemap *skybox;
Cubemap *skyboxIrradiance;
std::vector<Cubemap *> skyboxMipmaps;
std::vector<Cubemap *> skyboxRadiance;
Texture *brdfLutTex;

void LoadAssets()
{
	cubeMesh = MeshLoader::Load("assets\\cube.fbx");
	sphereMesh = MeshLoader::Load("assets\\sphere.fbx");
	planeMesh = MeshLoader::Load("assets\\plane.fbx");
	uvTex = TextureLoader::LoadPNG("assets\\texture.png");
	skybox = CubemapLoader::LoadVerticalEXR("assets\\skybox_default.exr");
	skyboxIrradiance = CubemapLoader::LoadVerticalEXR("assets\\skybox_default_irradiance_128K.exr");

	skyboxMipmaps.push_back(skybox);
	for (size_t i = 1; i <= 11; i++)
	{
		auto mipmap = CubemapLoader::LoadVerticalEXR(std::format("assets\\skybox_default_mipmap_{}.exr", i).c_str());
		skyboxMipmaps.push_back(mipmap);
	}

	skybox->SetMipmaps(skyboxMipmaps);

	skyboxRadiance.push_back(skybox);
	for (size_t i = 1; i <= 10; i++)
	{
		auto mipmap = CubemapLoader::LoadVerticalEXR(std::format("assets\\skybox_default_radiance_{}.exr", i).c_str());
		skyboxRadiance.push_back(mipmap);
	}

	skybox->SetRadianceMaps(skyboxRadiance);

	brdfLutTex = TextureLoader::LoadPNG("assets\\ibl_brdf_lut.png");
}

Scene *CreateScene()
{
	Scene *scene = new Scene();

#pragma region 立方体
	GameObject *cube = new GameObject();

#pragma region Transform
	Transform *meshTransform = new Transform();
	meshTransform->position = Eigen::Vector3f(2, 0, -10);
	meshTransform->scale = Eigen::Vector3f(1, 1, 1);
	meshTransform->Rotate(10, 0, 0);
	cube->AddComponent(meshTransform);
#pragma endregion

#pragma region MeshRenderer
	MeshRenderer *meshRenderer = new MeshRenderer(meshTransform);

	meshRenderer->mesh = cubeMesh;

	// 默认着色器
	auto *meshShader = new CookTorranceShader();
	meshShader->tex1 = uvTex;
	meshShader->irradianceTex = skyboxIrradiance;
	meshShader->radianceTex = skybox;
	meshShader->brdfLutTex = brdfLutTex;

	meshRenderer->shader = meshShader;
	cube->AddComponent(meshRenderer);
#pragma endregion

	scene->AddGameObject(cube);
#pragma endregion


#pragma region 球体
	GameObject *sphere = new GameObject();

#pragma region Transform
	Transform *sphereTransform = new Transform();
	sphereTransform->position = Eigen::Vector3f(-2, 0, -10);
	sphereTransform->scale = Eigen::Vector3f(1, 1, 1);
	sphere->AddComponent(sphereTransform);
#pragma endregion

#pragma region MeshRenderer
	MeshRenderer *sphereRenderer = new MeshRenderer(sphereTransform);
	sphereRenderer->mesh = sphereMesh;

	// 默认着色器
	auto *sphereShader = new CookTorranceShader();
	//sphereShader->tex1 = uvTex;
	sphereShader->irradianceTex = skyboxIrradiance;
	sphereShader->radianceTex = skybox;
	sphereShader->brdfLutTex = brdfLutTex;
	sphereShader->smoothness = 0.7f;
	sphereShader->metallic = 1.0f;
	sphereShader->diffuse << 0.7f, 0.7f, 0.75f, 1.0f;

	sphereRenderer->shader = sphereShader;
	sphere->AddComponent(sphereRenderer);
#pragma endregion

	scene->AddGameObject(sphere);
#pragma endregion


#pragma region 平面
	GameObject *plane = new GameObject();

#pragma region Transform
	Transform *planeTransform = new Transform();
	planeTransform->position = Eigen::Vector3f(0, -1, -10);
	planeTransform->scale = Eigen::Vector3f(9, 9, 9);
	planeTransform->Rotate(-90, 0, 0);
	plane->AddComponent(planeTransform);
#pragma endregion

#pragma region MeshRenderer
	MeshRenderer *planeRenderer = new MeshRenderer(planeTransform);

	planeRenderer->mesh = planeMesh;

	// 默认着色器
	auto *planeShader = new CookTorranceShader();
	//planeShader->tex1 = uvTex;
	planeShader->irradianceTex = skyboxIrradiance;
	planeShader->radianceTex = skybox;
	planeShader->brdfLutTex = brdfLutTex;

	planeRenderer->shader = planeShader;
	plane->AddComponent(planeRenderer);
#pragma endregion

	scene->AddGameObject(plane);
#pragma endregion

	return scene;
}

Buffer<TAAData> *InitTAABuffer(float width, float heigth)
{
	Buffer<TAAData> *taaBuffer = new Buffer<TAAData>(width, heigth);
	TAAData taaTemp;
	taaTemp.anchorColor = Color::MakeVector(Color::Black);
	for (size_t i = 0; i < MSAA_TYPE; i++)
	{
		taaTemp.subpixels[i].sampleCount = 0;
		taaTemp.subpixels[i].historyColor = Color::MakeVector(Color::Black);
	}
	taaBuffer->clear(taaTemp);

	return taaBuffer;
}

// 初始化空间，定义每个像素4个采样点
Framebuffer *InitFramebuffer(int width, int height)
{
	Framebuffer *framebuffer = new Framebuffer(width, height);

	// 定义子像素
	for (int x = 0; x < framebuffer->pixelBuffer.getWidth(); x++)
	{
		for (int y = 0; y < framebuffer->pixelBuffer.getHeight(); y++)
		{
			auto &pixelData = framebuffer->pixelBuffer.referPixel(x, y);
			pixelData.subpixels.resize(MSAA_TYPE);
		}
	}

	return framebuffer;
}

void ClearShadowMap(Framebuffer *shadowMap)
{
	// 必要的初始化
	for (int x = 0; x < shadowMap->pixelBuffer.getWidth(); x++)
	{
		for (int y = 0; y < shadowMap->pixelBuffer.getHeight(); y++)
		{
			auto &pixelData = shadowMap->pixelBuffer.referPixel(x, y);
			pixelData.subpixels.resize(1);

			auto screenPosition = Eigen::Vector2f(x + 0.5f, y + 0.5f);
			pixelData.subpixels[0].Reset(screenPosition);
		}
	}
}

void ClearFramebuffer(Framebuffer *framebuffer)
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
			auto &pixelData = framebuffer->pixelBuffer.referPixel(x, y);

			for (size_t subpixelIndex = 0; subpixelIndex < pixelData.subpixels.size(); subpixelIndex++)
			{
				auto screenPosition = Eigen::Vector2f(x + 0.5f, y + 0.5f);
				screenPosition += GetSubpixelPointBias(x, y, subpixelIndex) + bias;

				pixelData.subpixels[subpixelIndex].Reset(screenPosition);
			}
		}
	}
}

void Draw(DrawContext &context)
{
	Framebuffer *framebuffer = context.framebuffer;
	Scene *scene = context.scene;

	static Transform *cameraTransform = new Transform();
	//static OrthographicCamera* camera = new OrthographicCamera(cameraTransform, WIDTH / (float)HEIGHT);
	static PerspectiveCamera *camera = new PerspectiveCamera(cameraTransform, WIDTH / (float)HEIGHT);
	camera->transform->rotation = Eigen::Quaternionf::Identity();
	camera->transform->Rotate(-15, 0, 0);
	camera->transform->position = Eigen::Vector3f(0, 3, 0);

#if CAMERA_MOVE
	float moveSpeed = 1.0f;
	const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
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
	Graphics::SetTAABuffer(context.taaBuffer);

	static DirectionalLight *directionalLight = new DirectionalLight();
	directionalLight->direction = Eigen::Vector4f(-1, -1, -1, 0);

	Graphics::SetLight(directionalLight);

	Eigen::Vector4f ambientLightColor = Color::MakeVector(Color::White) * 0.25f;
	Graphics::SetAmbientLightColor(ambientLightColor);

	// 更新全屏插值数据
	PreDrawAllMeshes(scene, nullptr, DrawFlags::DrawFlags_ALL);

#pragma region 阴影绘制

	// 阴影相机
	auto sbb = scene->GetSphereBoudingBox();

	Eigen::Vector3f lightForward = directionalLight->direction.head<3>().normalized();

	Eigen::Vector3f tp = sbb.center - lightForward * (sbb.radius + camera->zNear + 1);
	//Graphics::DrawSphere(tp, 0.3f, Color::MakeVector(Color::Green));

	static Transform *shadowCameraTransform = new Transform();
	static OrthographicCamera *shadowCamera = new OrthographicCamera(shadowCameraTransform, WIDTH / (float)HEIGHT);

	shadowCamera->transform->position = sbb.center - lightForward * (sbb.radius + camera->zNear + 5);
	shadowCamera->transform->rotation = Eigen::Quaternionf::FromTwoVectors(
		-Eigen::Vector3f::UnitZ(),
		lightForward
	);
	shadowCamera->size = sbb.radius * 1.1f;

	// 阴影 Framebuffer
	ClearShadowMap(context.shadowMap);

	static InitShadowMapPass *initShadowMapPass = new InitShadowMapPass();
	initShadowMapPass->camera = shadowCamera;
	initShadowMapPass->shadowMap = context.shadowMap;
	Graphics::DrawPostprocessing(initShadowMapPass);

	Graphics::SetCamera(shadowCamera);
	Graphics::SetFramebuffer(context.shadowMap);

	static DepthTextureShader *depthTextureShader = new DepthTextureShader();
	PreDrawAllMeshes(scene, depthTextureShader, DrawFlags_ZBuffer);

	static MaximizeShadowMapPass *maximizeShadowMapPass = new MaximizeShadowMapPass();
	Graphics::DrawPostprocessing(maximizeShadowMapPass);

	Graphics::SetShadowMap(0, context.shadowMap, shadowCamera);

#pragma endregion

	Graphics::SetCamera(camera);
	Graphics::SetFramebuffer(framebuffer);

	context.directVisibilityMap[0]->clear(1);

	static DirectVisibilityMapPass *directVisibilityMapPass = new DirectVisibilityMapPass();
	directVisibilityMapPass->directVisibilityMap = context.directVisibilityMap[0];
	directVisibilityMapPass->shadowCamera = shadowCamera;
	directVisibilityMapPass->shadowMapBuffer = context.shadowMap;
	directVisibilityMapPass->light = directionalLight;
	Graphics::DrawPostprocessing(directVisibilityMapPass);

	Graphics::SetDirectVisibilityMap(0, context.directVisibilityMap[0]);

	// 绘制全屏
	Graphics::DrawFullScreen();

	static SkyboxShader *skyboxShader = new SkyboxShader();
	skyboxShader->tex1 = skybox;
	Graphics::DrawSkybox(skyboxShader);

#if !CAMERA_MOVE
	Graphics::DrawTAA();
#endif // !CAMERA_MOVE

	//Graphics::DrawSphere(sbb.center, sbb.radius, Color::MakeVector(Color::Red));

	Graphics::MergeSubpixelsAndWrite();

	DrawPixelData(context.shadowMap, &(context.framebuffer->colorBuffer), 128, 128);
	//framebuffer->colorBuffer.drawImage(context.shadowMap->colorBuffer.data(),
	//    context.shadowMap->getWidth(), 
	//    context.shadowMap->getHeight(), 128, 128);
	framebuffer->colorBuffer.drawLine(Eigen::Vector2f(0, 0),
		Eigen::Vector2f(40, 30), Color::Yellow);

	Graphics::Clear();
}

void PreDrawAllMeshes(Scene *scene, Shader *shader, DrawFlags drawFlags)
{
	for (size_t i = 0; i < scene->gameObjects.data.size(); i++)
	{
		if (!scene->gameObjects.valid[i])
		{
			continue;
		}

		auto gameObject = scene->gameObjects.data[i];

		MeshRenderer *renderer = gameObject->GetComponent<MeshRenderer>();

		if (renderer == nullptr)
		{
			continue;
		}

		Transform *transform = gameObject->GetComponent<Transform>();

		Graphics::PreDrawMesh(renderer->mesh, transform->GetModelMatrix(), shader ? shader : renderer->shader, drawFlags);
	}
}

void DrawPixelData(Framebuffer *framebuffer, Colorbuffer *colorbuffer, float height, float width)
{
	auto imageWidth = framebuffer->getWidth();
	auto imageHeight = framebuffer->getHeight();

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {

			int rx = static_cast<int>(static_cast<float>(x) * imageWidth / width);
			int ry = static_cast<int>(static_cast<float>(y) * imageHeight / height);

			rx = MathUtils::Clamp(rx, 0, imageWidth - 1);
			ry = MathUtils::Clamp(ry, 0, imageHeight - 1);

			auto &pixelData = framebuffer->pixelBuffer.referPixel(rx, ry);

			Eigen::Vector4f color = Eigen::Vector4f::Zero();
			for (size_t i = 0; i < pixelData.subpixels.size(); i++)
			{
				auto &subpixelData = pixelData.subpixels[i];

				Eigen::Vector4f subColor = Eigen::Vector4f::Ones() * subpixelData.z;
				subColor.w() = 1;

				color += subColor / pixelData.subpixels.size();
			}

			colorbuffer->putPixel(x, colorbuffer->height - 1 - y, Color::Make(color));
		}
	}
}