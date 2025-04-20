#include "LatitudeLongitudeMapLoader.h"  
#include <ImfOutputFile.h>  
#include <ImfChannelList.h>  
#include <ImfFrameBuffer.h>  
#include <ImfHeader.h> // Include this to resolve incomplete type "Imf_3_3::Header"  
#include <ImfInputFile.h> // Add this include to resolve incomplete type "Imf_3_3::InputFile"  
#include <ImfRgbaFile.h>  // Ensure all necessary OpenEXR headers are included  
#include <stdexcept>  
#include "MathUtils.h"  
#include "LatitudeLongitudeMap.h"  

void LatitudeLongitudeMapLoader::SaveLatitudeLongitudeEXR(const char *path, const LatitudeLongitudeMap *latitudeLongitudeMap)  
{  
   try {  
       const int width = latitudeLongitudeMap->width;  
       const int height = latitudeLongitudeMap->height;  

       // Initialize EXR file header and set data window  
       Imf::Header header(width, height);  
       Imath::Box2i dataWindow(Imath::V2i(0, 0), Imath::V2i(width - 1, height - 1));  
       header.dataWindow() = dataWindow;  
       header.displayWindow() = dataWindow;  

       // Add RGBA channels (32-bit float)  
       header.channels().insert("R", Imf::Channel(Imf::FLOAT));  
       header.channels().insert("G", Imf::Channel(Imf::FLOAT));  
       header.channels().insert("B", Imf::Channel(Imf::FLOAT));  
       header.channels().insert("A", Imf::Channel(Imf::FLOAT));  

       // Create output file  
       Imf::OutputFile file(path, header);  

       // Prepare pixel data buffer (row-major storage, Y-axis flipped)  
       std::vector<float> pixelBuffer(width * height * 4);  

       // Fill buffer and flip Y-axis  
       for (int y = 0; y < height; ++y) {  
           for (int x = 0; x < width; ++x) {  
               const int flippedY = height - 1 - y;  
               const Eigen::Vector4f &color = latitudeLongitudeMap->data[y * width + x];  
               const size_t bufferIndex = (flippedY * width + x) * 4;  

               pixelBuffer[bufferIndex] = color[0];  // R  
               pixelBuffer[bufferIndex + 1] = color[1];  // G  
               pixelBuffer[bufferIndex + 2] = color[2];  // B  
               pixelBuffer[bufferIndex + 3] = color[3];  // A  
           }  
       }  

       // Configure FrameBuffer  
       Imf::FrameBuffer frameBuffer;  
       char *base = reinterpret_cast<char *>(pixelBuffer.data());  
       const size_t xStride = 4 * sizeof(float);  
       const size_t yStride = width * 4 * sizeof(float);  

       frameBuffer.insert("R", Imf::Slice(Imf::FLOAT, base, xStride, yStride));  
       frameBuffer.insert("G", Imf::Slice(Imf::FLOAT, base + sizeof(float), xStride, yStride));  
       frameBuffer.insert("B", Imf::Slice(Imf::FLOAT, base + 2 * sizeof(float), xStride, yStride));  
       frameBuffer.insert("A", Imf::Slice(Imf::FLOAT, base + 3 * sizeof(float), xStride, yStride));  

       // Write to file  
       file.setFrameBuffer(frameBuffer);  
       file.writePixels(height);  
   }  
   catch (const std::exception &e) {  
       throw std::runtime_error("EXR save failed: " + std::string(e.what()));  
   }  
}  

LatitudeLongitudeMap *LatitudeLongitudeMapLoader::LoadLatitudeLongitudeMapEXR(const char *path)  
{  
   LatitudeLongitudeMap *map = nullptr;  
   try {  
       Imf::InputFile file(path);  
       const Imf::Header &header = file.header();  
       const Imath::Box2i &dw = header.dataWindow();  
       const int width = dw.max.x - dw.min.x + 1;  
       const int height = dw.max.y - dw.min.y + 1;  

       map = new LatitudeLongitudeMap(width, height);  
       std::vector<float> pixelBuffer(width * height * 4);  

       // Configure FrameBuffer  
       Imf::FrameBuffer frameBuffer;  
       char *base = reinterpret_cast<char *>(pixelBuffer.data());  
       const size_t xStride = 4 * sizeof(float);  
       const size_t yStride = width * 4 * sizeof(float);  

       // Handle Alpha channel  
       const Imf::ChannelList &channels = header.channels();  
       const bool hasAlpha = channels.findChannel("A");  

       frameBuffer.insert("R", Imf::Slice(Imf::FLOAT, base, xStride, yStride));  
       frameBuffer.insert("G", Imf::Slice(Imf::FLOAT, base + sizeof(float), xStride, yStride));  
       frameBuffer.insert("B", Imf::Slice(Imf::FLOAT, base + 2 * sizeof(float), xStride, yStride));  
       if (hasAlpha) {  
           frameBuffer.insert("A", Imf::Slice(Imf::FLOAT, base + 3 * sizeof(float), xStride, yStride));  
       }  
       else {  
           std::fill(pixelBuffer.begin() + 3, pixelBuffer.end(), 1.0f); // Fill Alpha with 1  
       }  

       // Read pixel data  
       file.setFrameBuffer(frameBuffer);  
       file.readPixels(dw.min.y, dw.max.y);  

       // Convert data and fix orientation  
       for (int y = 0; y < height; ++y) {  
           const int targetY = height - 1 - y; // Y-axis flipped  
           for (int x = 0; x < width; ++x) {  
               const size_t srcIdx = (y * width + x) * 4;  
               Eigen::Vector4f color(  
                   pixelBuffer[srcIdx],  
                   pixelBuffer[srcIdx + 1],  
                   pixelBuffer[srcIdx + 2],  
                   hasAlpha ? pixelBuffer[srcIdx + 3] : 1.0f  
               );  

               MathUtils::RemoveNan(color, 500);
               MathUtils::ClampVector4(color, 0, 500);
               map->PutPixel(x, targetY, color);  
           }  
       }  
       return map;  
   }  
   catch (const std::exception &e) {  
       delete map;  
       throw std::runtime_error("EXR load failed: " + std::string(e.what()));  
   }  
}