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

float GetGgx(Eigen::Vector4f &halfVector, Eigen::Vector4f &normal, float roughness)
{
    float alpha = roughness * roughness;
	float hDotN = halfVector.dot(normal);
    float k = (alpha - 1) * hDotN * hDotN + 1;
    return alpha / (M_PI * std::powf(k, 2));
}

// https://learnopengl-cn.github.io/07%20PBR/03%20IBL/02%20Specular%20IBL/
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float a = (roughness + 1) / 2;  // 解析光源特殊处理
    float k = (a * a) / 2.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// https://learnopengl-cn.github.io/07%20PBR/03%20IBL/02%20Specular%20IBL/
float GeometrySmith(Eigen::Vector4f N, Eigen::Vector4f V, Eigen::Vector4f L, float roughness)
{
    float NdotV = std::max(N.dot(V), 0.0f);
    float NdotL = std::max(N.dot(L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
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

    // 垂直入射反射率
    auto F0 = MathUtils::Lerp(vec4(0.04f), albedo, metallic);
	float roughness = 1 - smoothness;
    Eigen::Vector4f F = F0 + (vec4(1) - F0) * std::powf(1 - vDotN, 5);

	Eigen::Vector4f R = MathUtils::Reflect(viewDirection, worldNormal).normalized();

    Eigen::Vector4f kd = (vec4(1) - F) * (1 - metallic);

    //Eigen::Vector4f diffuse = Eigen::Vector4f::Zero();
    //for (size_t i = 0; i < context->directionalLightDatas.size(); i++)
    //{
    //    auto &lightData = context->directionalLightDatas[i];

    //    auto diffuseIntensity = MathUtils::Clamp(worldNormal.dot(-lightData.worldSpaceDirection), 0.0f, 1.0f);
    //    Eigen::Vector4f color = lightData.color * diffuseIntensity * ShaderUtils::GetVisibility(context, lightData.lightIndex);
    //    diffuse += color;
    //}
    //diffuse = diffuse.cwiseProduct(albedo);

    // pbr 直接光
    Eigen::Vector4f directColor = Eigen::Vector4f::Zero();
    for (size_t i = 0; i < context->directionalLightDatas.size(); i++)
    {
        auto &lightData = context->directionalLightDatas[i];

        Eigen::Vector4f lightDirection = -lightData.worldSpaceDirection;

        Eigen::Vector4f halfVector = (viewDirection - lightData.worldSpaceDirection).normalized();

        auto diffuseIntensity = MathUtils::Clamp(worldNormal.dot(-lightData.worldSpaceDirection), 0.0f, 1.0f);
        
        Eigen::Vector4f diffuse = (lightData.color * diffuseIntensity).cwiseProduct(albedo).cwiseProduct(kd);
        
        float D = GetGgx(halfVector, worldNormal, roughness);
        float G = GeometrySmith(worldNormal, viewDirection, lightDirection, roughness);
        Eigen::Vector4f specularBrdf = F * D * G / (4 * worldNormal.dot(viewDirection));
        Eigen::Vector4f specular = lightData.color.cwiseProduct(specularBrdf);
        
        directColor += (diffuse + specular) *ShaderUtils::GetVisibility(context, lightData.lightIndex);
    }

    // pbr 间接光
    auto indirectDiffuse = irradianceTex->Sample(worldNormal.head<3>());
    indirectDiffuse = indirectDiffuse.cwiseProduct(albedo).cwiseProduct(kd);

    auto indirectSpecular = radianceTex->SampleByRoughness(R.head<3>(), roughness);
    
    auto scaleAndBias = brdfLutTex->Sample(vDotN, roughness);
    indirectSpecular = indirectSpecular.cwiseProduct(F0 * scaleAndBias.x() + vec4(scaleAndBias.y()));

    // 综合颜色
    Eigen::Vector4f color = directColor + indirectDiffuse + indirectSpecular;
    
    return color;
}
