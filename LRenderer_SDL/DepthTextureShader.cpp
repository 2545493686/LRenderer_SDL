#include "DepthTextureShader.h"

v2f DepthTextureShader::vertex(const appdata& v)
{
    v2f o;

    o.vertex = ShaderUtils::ToClipPos(context, v.vertex);

    usedTexCount = 1;
    o.texcoords[0] = ShaderUtils::ToViewPos(context, v.vertex);
    
    return o;
}

Eigen::Vector4f DepthTextureShader::fragment(const v2f& i)
{
    Eigen::Vector4f result = Eigen::Vector4f::Ones();
    result *= -i.texcoords[0].z() / context->zFar;
    result.w() = 1;
    return result;
}
