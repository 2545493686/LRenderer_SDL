#include "GrayscaleOperator.h"
#include <Eigen/Dense>

void GrayscaleOperator::Invoke(Texture *tex)
{	// »Ò¶È»¯
	tex->Each([](int x, int y, Eigen::Vector4f &value)
	{
		value = Eigen::Vector4f::Ones() * (value.x() * 0.299f + value.y() * 0.587f + value.z() * 0.114f);
	});
}