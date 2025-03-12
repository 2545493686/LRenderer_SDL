#include "GraphicsType.h"

SubpixelData::SubpixelData(Eigen::Vector2f screenPosition)
{
    Reset(screenPosition);
}

void SubpixelData::Reset(Eigen::Vector2f screenPosition)
{
    this->screenPosition = screenPosition;
    color = Color::MakeVector(Color::Black);
    shader = nullptr;
    z = std::numeric_limits<float>::max();
    shadowSubIndex = -1;
}