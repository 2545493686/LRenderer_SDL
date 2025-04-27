#include "DerivativeOperator.h"
#include "Eigen/Dense"

void DerivativeOperator::Invoke(Texture *tex)
{
	dx = new Texture(tex->width, tex->height);
	dy = new Texture(tex->width, tex->height);

	auto temp = tex->Copy();
	tex->Each([this, tex, temp](int x, int y, Eigen::Vector4f &value)
	{
		int nextX = x + 1;
		int nextY = y + 1;

		tex->LegalizationCoordinates(nextX, nextY, Texture::WrapPingPong);

		Eigen::Vector4f vdx = (temp->ReferDirect(nextX, y) - temp->ReferDirect(x, y));
		Eigen::Vector4f vdy = (temp->ReferDirect(x, nextY) - temp->ReferDirect(x, y));
		this->dx->ReferDirect(x, y) = vdx;
		this->dy->ReferDirect(x, y) = vdy;
		
		tex->ReferDirect(x, y) = Eigen::Vector4f::Ones() * std::sqrtf(std::powf(vdx.x(), 2) + std::powf(vdx.y(), 2));
	});
}


