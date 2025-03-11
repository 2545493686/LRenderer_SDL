#pragma once
#include "Shader.h"

// v2f 
// vertex：world space position

class BuiltinShader : public Shader
{
    virtual v2f vertex(const appdata& v) override;
    virtual Eigen::Vector4f fragment(const v2f& i) override;
};

