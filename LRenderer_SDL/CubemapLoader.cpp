#include "CubemapLoader.h"

// deepseek 生成
Cubemap* CubemapLoader::LoadVerticalEXR(const char* path)
{
    auto validateFile = [](const Imf::Header& header) {
        const Imath::Box2i& dw = header.dataWindow();
        const int width = dw.max.x - dw.min.x + 1;
        const int height = dw.max.y - dw.min.y + 1;
        return (height == 6 * width);
    };

    try {
        Imf::InputFile file(path);
        if (!validateFile(file.header())) {
            throw std::runtime_error("Invalid vertical cubemap layout");
        }

        const Imath::Box2i& dw = file.header().dataWindow();
        const int size = dw.max.x - dw.min.x + 1;

        auto cubemap = new Cubemap(size);
        LoadDataToCubemap(file, *cubemap);

        return cubemap;

    }
    catch (const std::exception& e) {
        throw std::runtime_error("Cubemap loading failed: " + std::string(e.what()));
    }
}

// deepseek 生成
void CubemapLoader::LoadDataToCubemap(Imf::InputFile& file, Cubemap& cubemap)
{
    const Imath::Box2i& dw = file.header().dataWindow();
    const int size = cubemap.size;
    const int totalHeight = dw.max.y - dw.min.y + 1;

    std::vector<float> pixelBuffer(6 * size * size * 4);

    // 配置FrameBuffer
    Imf::FrameBuffer frameBuffer;
    frameBuffer.insert("R", Imf::Slice(Imf::FLOAT,
        (char*)(pixelBuffer.data() + 0), 4 * sizeof(float), 4 * size * sizeof(float)));
    frameBuffer.insert("G", Imf::Slice(Imf::FLOAT,
        (char*)(pixelBuffer.data() + 1), 4 * sizeof(float), 4 * size * sizeof(float)));
    frameBuffer.insert("B", Imf::Slice(Imf::FLOAT,
        (char*)(pixelBuffer.data() + 2), 4 * sizeof(float), 4 * size * sizeof(float)));
    frameBuffer.insert("A", Imf::Slice(Imf::FLOAT,
        (char*)(pixelBuffer.data() + 3), 4 * sizeof(float), 4 * size * sizeof(float)));

    file.setFrameBuffer(frameBuffer);
    file.readPixels(dw.min.y, dw.max.y);

    // 填充到Cubemap
    for (int face = 0; face < 6; ++face) {
        const int verticalOffset = face * size;
        Eigen::Vector4f* faceData = cubemap.data[face];

        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                //const int srcY = totalHeight - 1 - (verticalOffset + y);
                const int srcY = verticalOffset + y;
                const int srcIndex = srcY * size + x;
                const int dstIndex = y * size + x;

                faceData[dstIndex] = Eigen::Vector4f(
                    std::pow(pixelBuffer[4 * srcIndex + 0], 2.2f),
                    std::pow(pixelBuffer[4 * srcIndex + 1], 2.2f),
                    std::pow(pixelBuffer[4 * srcIndex + 2], 2.2f),
                    pixelBuffer[4 * srcIndex + 3]
                );
            }
        }
    }
}
