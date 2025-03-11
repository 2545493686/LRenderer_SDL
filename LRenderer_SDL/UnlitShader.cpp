#include "UnlitShader.h"

v2f UnlitShader::vertex(const appdata &v)
{
	v2f o;
	o.vertex = ShaderUtils::ToClipPos(context, v.vertex);
	
	usedTexCount = 1;
	o.texcoords[0] = Eigen::Vector4f(v.uv0.x(), v.uv0.y(), 0, 0);
	return o;
}

Eigen::Vector4f UnlitShader::fragment(const v2f& i)
{
	//Eigen::Vector4f color = i.texcoords[0];
	//color.w() = 1;
	Eigen::Vector4f color = tex1->Sample(i.texcoords[0].x(), i.texcoords[0].y());
	return color;
}
