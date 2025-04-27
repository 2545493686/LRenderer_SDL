#include "SobelOperator.h"

#include "Eigen/Dense"

void SobelOperator::Invoke(Texture *tex)
{
	Eigen::MatrixXf gx(3, 3);
	gx << -1, 0, 1,
		-2, 0, 2,
		-1, 0, 1;

	Eigen::MatrixXf gy(3, 3);
	gy = gx.transpose();

	dx = tex->Filter(gx);
	dy = tex->Filter(gy);
}
