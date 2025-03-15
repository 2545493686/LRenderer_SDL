#include "BuiltinShader.h"

// 输入 v.vertex 是模型空间坐标
// 输出 o.vertex 是世界空间坐标
// 输出 o.texcoords[0] 是世界空间法向量，已归一化
v2f BuiltinShader::vertex(const appdata& v)
{
    v2f o;
    
    o.vertex = ShaderUtils::ToWorldPos(context, v.vertex);
    usedTexCount = 1;

    o.texcoords[0] = ShaderUtils::ToWorldPos(context, v.normal).normalized();

    return o;
}

// 不支持
Eigen::Vector4f BuiltinShader::fragment(const v2f& i)
{
	return Eigen::Vector4f();
}
