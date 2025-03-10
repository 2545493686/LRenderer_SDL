#include "SkyboxShader.h"

v2f SkyboxShader::vertex(const appdata& v)
{
    v2f o;

    o.vertex = ShaderUtils::ToClipPos(context, v.vertex);
    o.texcoords[0] = ShaderUtils::ToWorldPos(context, v.vertex);

    return o;
}

Eigen::Vector4f SkyboxShader::fragment(const v2f& i)
{
    Eigen::Vector3f f = i.texcoords[0].head<3>() - context->cameraWorldPos;

    auto color = tex1->Sample(f);;
    return color * 1.2f;
}
