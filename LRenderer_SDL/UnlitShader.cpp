#include "UnlitShader.h"

v2f UnlitShader::vertex(const appdata &v)
{
	v2f o;
	o.vertex = ShaderUtils::ToClipPos(context, v.vertex);
	o.texcoords[0] = Eigen::Vector4f(v.uv0.x(), v.uv0.y(), 0, 0);
	return o;
}

Eigen::Vector4f UnlitShader::fragment(const v2f& i)
{
	return Eigen::Vector4f(1, 1, 1, 1);
}
