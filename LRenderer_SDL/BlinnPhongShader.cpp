#include "BlinnPhongShader.h"

v2f BlinnPhongShader::vertex(const appdata& v)
{
    v2f o;

    o.vertex = ShaderUtils::ToClipPos(context, v.vertex);
    o.texcoords[0] << v.uv0.x(), v.uv0.y(), 0, 0;
    o.texcoords[1] = ShaderUtils::ToClipPos(context, v.normal);

    return o;
}

Eigen::Vector4f BlinnPhongShader::fragment(const v2f& i)
{
    Eigen::Vector4f worldNormal = i.texcoords[1].normalized();

    Eigen::Vector4f baseColor = tex1->Sample(i.texcoords[0].x(), i.texcoords[0].y());

    Eigen::Vector4f diffuse = Eigen::Vector4f::Zero();
    for (size_t i = 0; i < context->directionalLightCount; i++)
    {
        auto & lightData = context->directionalLightDatas[i];

        auto diffuseIntensity = MathUtils::Clamp(worldNormal.dot(-lightData.worldSpaceDirection), 0.0f, 1.0f);
        Eigen::Vector4f color = lightData.color * diffuseIntensity;
        diffuse += color;
    }
    diffuse = diffuse.cwiseProduct(this->diffuse);

    Eigen::Vector4f color = diffuse + context->ambientLightColor;

    color = color.cwiseProduct(baseColor);

    return color;
}