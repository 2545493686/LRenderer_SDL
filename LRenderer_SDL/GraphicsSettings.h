#pragma once

constexpr auto MSAA_X4 = 4;
constexpr auto MSAA_TYPE = MSAA_X4;

constexpr auto V2F_TEX_COUNT = 6;

#define BOOT_GAME 0
#define BOOT_IRRADIANCE_BAKER 1
#define BOOT_MODE BOOT_IRRADIANCE_BAKER

#define SUBPIXEL_BIAS 1
#define DEBUG_COUNT 0
#define CAMERA_MOVE 0

class GraphicsSettings
{
};

