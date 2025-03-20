#pragma once

#include <vector>

#include <ImfInputFile.h>
#include <ImfFrameBuffer.h>
#include <ImathBox.h>

#include "Cubemap.h"

class CubemapLoader {
public:
    // 输入文件是纵向排列的6张Cubemap
    // x+  x-  y+  y-  z+  z-
    static Cubemap *LoadVerticalEXR(const char* path);
    static void SaveVerticalEXR(const char *path, const Cubemap *cubemap);
    
private:
    static void LoadDataToCubemap(Imf::InputFile& file, Cubemap& cubemap);
};
