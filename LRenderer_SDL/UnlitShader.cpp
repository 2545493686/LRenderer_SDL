#include "UnlitShader.h"

v2f* UnlitShader::vertex(const appdata* v)
{
	v2f* o = new v2f;
	o->vertex = ShaderUtils::ToClipPos(context, v->vertex);
	return o;
}

Eigen::Vector4f UnlitShader::fragment(const v2f* i)
{
	return Eigen::Vector4f(1, 1, 1, 1);
}
