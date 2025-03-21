#include "CookTorranceShader.h"

#include <cmath>
#include "Cubemap.h"
#include "MathUtils.h"

v2f CookTorranceShader::vertex(const appdata &v)
{
    v2f o;

    o.vertex = ShaderUtils::ToClipPos(context, v.vertex);

    usedTexCount = 1;
    o.texcoords[0] << v.uv0.x(), v.uv0.y(), 0, 0;
    
    return o;
}

Eigen::Vector4f GetFresnelTerm(const Eigen::Vector4f &F0, const Eigen::Vector4f &v, const Eigen::Vector4f &n)
{
    return F0 + (vec4(1) - F0) * std::pow(1 - v.dot(n), 5);
}

Eigen::Vector4f CookTorranceShader::fragment(const v2f &i)
{
    Eigen::Vector4f worldNormal = context->worldNormal;
    Eigen::Vector4f viewDirection = (context->cameraWorldPos - context->worldPosition).normalized();

    float vDotN = std::max(viewDirection.dot(worldNormal), 0.0f);

    Eigen::Vector4f albedo = this->diffuse;
    if (tex1)
    {
        albedo = tex1->Sample(i.texcoords[0].x(), i.texcoords[0].y());
    }

    Eigen::Vector4f diffuse = Eigen::Vector4f::Zero();
    for (size_t i = 0; i < context->directionalLightDatas.size(); i++)
    {
        auto &lightData = context->directionalLightDatas[i];

        auto diffuseIntensity = MathUtils::Clamp(worldNormal.dot(-lightData.worldSpaceDirection), 0.0f, 1.0f);
        Eigen::Vector4f color = lightData.color * diffuseIntensity * ShaderUtils::GetVisibility(context, lightData.lightIndex);
        diffuse += color;
    }
    diffuse = diffuse.cwiseProduct(albedo);

    // pbr 间接光

    // 垂直入射反射率
    auto F0 = MathUtils::Lerp(vec4(0.04f), albedo, metallic);
    Eigen::Vector4f F = F0 + (vec4(1) - F0) * std::powf(1 - vDotN, 5);
	float roughness = 1 - smoothness;
	Eigen::Vector4f R = MathUtils::Reflect(viewDirection, worldNormal).normalized();

    Eigen::Vector4f kd = (vec4(1) - F) * (1 - metallic);
    
    auto indrirectDiffuse = irradianceTex->Sample(worldNormal.head<3>());
    indrirectDiffuse = indrirectDiffuse.cwiseProduct(albedo).cwiseProduct(kd);

    auto indrirectSpecular = radianceTex->SampleByRoughness(R.head<3>(), roughness);
    
    auto scaleAndBias = brdfLutTex->Sample(vDotN, 0);
    indrirectSpecular = indrirectSpecular.cwiseProduct(F0 * scaleAndBias.x() + vec4(scaleAndBias.y()));

    // 综合颜色
    Eigen::Vector4f color = diffuse + indrirectDiffuse + indrirectSpecular;
    
    return color;
}
