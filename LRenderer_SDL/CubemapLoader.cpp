#include "CubemapLoader.h"

#include <format>
#include <cmath>
#include <string>

// deepseek 生成
#include "CubemapLoader.h"
#include <ImfChannelList.h> // Add this include
#include <ImfOutputFile.h>  // Add this include
#include <ImfStringAttribute.h>

#include "MathUtils.h"

// deepseek 生成
void CubemapLoader::SaveVerticalEXR(const char *path, const Cubemap *cubemap)
{
    try {
        const int size = cubemap->size;
        const int totalHeight = 6 * size;

        // 创建EXR文件头并设置数据窗口
        Imf::Header header(size, totalHeight);
        Imath::Box2i dataWindow(Imath::V2i(0, 0), Imath::V2i(size - 1, totalHeight - 1));
        header.dataWindow() = dataWindow;
        header.displayWindow() = dataWindow;

        // 在头文件中添加方向属性
        Imf::StringAttribute faceDirections("+X,-X,+Y,-Y,+Z,-Z"); // 根据实际顺序调整
        header.insert("faceDirections", faceDirections);

        // 添加RGBA通道（32位浮点）
        header.channels().insert("R", Imf::Channel(Imf::FLOAT));
        header.channels().insert("G", Imf::Channel(Imf::FLOAT));
        header.channels().insert("B", Imf::Channel(Imf::FLOAT));
        header.channels().insert("A", Imf::Channel(Imf::FLOAT));

        // 创建输出文件
        Imf::OutputFile file(path, header);

        // 准备像素数据缓冲区（行优先存储）
        std::vector<float> pixelBuffer(totalHeight * size * 4);

        // 将Cubemap的六个面数据填充到缓冲区
        for (int face = 0; face < 6; ++face) {
            const Eigen::Vector4f *faceData = cubemap->data[face];
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    // 计算缓冲区中的索引（垂直排列）
                    const int yTotal = face * size + y;
                    const int idx = (yTotal * size + x) * 4;
                    const Eigen::Vector4f &pixel = faceData[y * size + x];

                    pixelBuffer[idx + 0] = pixel[0]; // R
                    pixelBuffer[idx + 1] = pixel[1]; // G
                    pixelBuffer[idx + 2] = pixel[2]; // B
                    pixelBuffer[idx + 3] = pixel[3]; // A
                }
            }
        }

        // 配置FrameBuffer
        Imf::FrameBuffer frameBuffer;
        char *base = reinterpret_cast<char *>(pixelBuffer.data());
        const size_t xStride = 4 * sizeof(float);    // 像素间步长
        const size_t yStride = size * 4 * sizeof(float); // 行间步长

        frameBuffer.insert("R", Imf::Slice(Imf::FLOAT, base + 0 * sizeof(float), xStride, yStride));
        frameBuffer.insert("G", Imf::Slice(Imf::FLOAT, base + 1 * sizeof(float), xStride, yStride));
        frameBuffer.insert("B", Imf::Slice(Imf::FLOAT, base + 2 * sizeof(float), xStride, yStride));
        frameBuffer.insert("A", Imf::Slice(Imf::FLOAT, base + 3 * sizeof(float), xStride, yStride));

        // 写入文件
        file.setFrameBuffer(frameBuffer);
        file.writePixels(totalHeight); // 写入所有扫描线

    }
    catch (const std::exception &e) {
        throw std::runtime_error("Failed to save vertical cubemap EXR: " + std::string(e.what()));
    }
}

void CubemapLoader::SaveLatitudeLongitudeEXR(const char *path, const Cubemap *cubemap)
{
    try {
        const int size = cubemap->size;
        const int width = size;  // 纬度经度图的宽高比为2:1
        const int height = size / 2;

        // 创建EXR文件头
        Imf::Header header(width, height);
        Imath::Box2i dataWindow(Imath::V2i(0, 0), Imath::V2i(width - 1, height - 1));
        header.dataWindow() = dataWindow;
        header.displayWindow() = dataWindow;

        // 添加RGBA通道（32位浮点）
        header.channels().insert("R", Imf::Channel(Imf::FLOAT));
        header.channels().insert("G", Imf::Channel(Imf::FLOAT));
        header.channels().insert("B", Imf::Channel(Imf::FLOAT));
        header.channels().insert("A", Imf::Channel(Imf::FLOAT));

        // 创建输出文件
        Imf::OutputFile file(path, header);

        // 准备像素数据缓冲区
        std::vector<float> pixelBuffer(width * height * 4);

        // 常量定义
        const float pi = 3.14159265358979323846f;
        const float twoPi = 2.0f * pi;
        const float halfPi = pi / 2.0f;

        // 遍历每个像素生成纬度经度映射
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // 计算球面坐标（phi经度，theta纬度）
                const float u = (x + 0.5f) / width;  // 规范化到[0,1)
                const float v = 1.0f - (y + 0.5f) / height; // 翻转Y轴

                const float phi = u * twoPi + pi / 2;         // 经度 0-2π
                const float theta = v * pi - halfPi; // 纬度 -π/2到π/2

                // 转换为笛卡尔坐标
                const float cosTheta = cos(theta);
                const float sinTheta = sin(theta);
                const float cosPhi = cos(phi);
                const float sinPhi = sin(phi);

                // 生成方向向量（右手坐标系，Y-up）
                Eigen::Vector3f direction(
                    cosTheta * cosPhi,
                    sinTheta,
                    cosTheta * sinPhi
                );

                // 使用双线性采样获取颜色
                Eigen::Vector4f color = cubemap->Sample(direction, Cubemap::SampleType::Bilinear);

                // 存储到缓冲区
                const int idx = (y * width + x) * 4;
                pixelBuffer[idx + 0] = color[0]; // R
                pixelBuffer[idx + 1] = color[1]; // G
                pixelBuffer[idx + 2] = color[2]; // B
                pixelBuffer[idx + 3] = color[3]; // A
            }
        }

        // 配置FrameBuffer
        Imf::FrameBuffer frameBuffer;
        char *base = reinterpret_cast<char *>(pixelBuffer.data());
        const size_t xStride = 4 * sizeof(float);
        const size_t yStride = width * 4 * sizeof(float);

        frameBuffer.insert("R", Imf::Slice(Imf::FLOAT, base + 0 * sizeof(float), xStride, yStride));
        frameBuffer.insert("G", Imf::Slice(Imf::FLOAT, base + 1 * sizeof(float), xStride, yStride));
        frameBuffer.insert("B", Imf::Slice(Imf::FLOAT, base + 2 * sizeof(float), xStride, yStride));
        frameBuffer.insert("A", Imf::Slice(Imf::FLOAT, base + 3 * sizeof(float), xStride, yStride));

        // 写入文件
        file.setFrameBuffer(frameBuffer);
        file.writePixels(height);
    }
    catch (const std::exception &e) {
        throw std::runtime_error("Failed to save latitude longitude EXR: " + std::string(e.what()));
    }
}

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

        cubemap->mipmaps = std::vector<Cubemap *>();
        cubemap->mipmaps.push_back(cubemap);

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
                    pixelBuffer[4 * srcIndex + 0],
                    pixelBuffer[4 * srcIndex + 1],
                    pixelBuffer[4 * srcIndex + 2],
                    pixelBuffer[4 * srcIndex + 3]
                );

                if (faceData[dstIndex][3] == 0)
                {
                    faceData[dstIndex][3] = 1;
                }

                MathUtils::RemoveNan(faceData[dstIndex], 10);
                MathUtils::ClampVector4(faceData[dstIndex], 0, 10);
            }
        }
    }
}
