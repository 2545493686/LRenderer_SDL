#include "BuiltinShader.h"

v2f BuiltinShader::vertex(const appdata& v)
{
    v2f o;

    o.vertex = ShaderUtils::ToWorldPos(context, v.vertex);
    usedTexCount = 0;

    return o;
}

Eigen::Vector4f BuiltinShader::fragment(const v2f& i)
{
	return Eigen::Vector4f();
}
