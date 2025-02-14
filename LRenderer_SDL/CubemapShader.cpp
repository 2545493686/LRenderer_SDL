#include "CubemapShader.h"

v2f CubemapShader::vertex(const appdata& v)
{
	v2f o;
	o.vertex = ShaderUtils::ToClipPos(context, v.vertex);
	o.texcoords[0] = v.vertex;

	return o;
}

Eigen::Vector4f CubemapShader::fragment(const v2f& i)
{
    Eigen::Vector3f direction = i.texcoords[0].head<3>();

	return cubemap->Sample(direction);
}
