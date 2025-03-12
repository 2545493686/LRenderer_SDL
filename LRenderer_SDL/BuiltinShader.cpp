#include "BuiltinShader.h"

// 输入 v.vertex 是裁剪空间坐标
// 输出 o.vertex 是世界空间坐标
v2f BuiltinShader::vertex(const appdata& v)
{
    v2f o;
    
    o.vertex = context->clipToWorldMatrix * v.vertex;
    usedTexCount = 0;

    return o;
}

Eigen::Vector4f BuiltinShader::fragment(const v2f& i)
{
	return Eigen::Vector4f();
}
