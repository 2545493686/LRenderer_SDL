#include "SkyboxShader.h"

v2f SkyboxShader::vertex(const appdata& v)
{
    v2f o;

    o.vertex = ShaderUtils::ToClipPos(context, v.vertex);
    
    usedTexCount = 0;
    //o.texcoords[0] = ShaderUtils::ToWorldPos(context, v.vertex);

    return o;
}

Eigen::Vector4f SkyboxShader::fragment(const v2f& i)
{
    //Eigen::Vector3f f = i.texcoords[0].head<3>() - context->cameraWorldPos;
    Eigen::Vector4f f = context->worldPosition - context->cameraWorldPos;

    auto color = tex1->Sample(f.head<3>());;

    color = color.array().pow(1.8f);
    return color;
}
