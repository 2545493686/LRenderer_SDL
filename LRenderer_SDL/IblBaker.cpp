#include "IblBaker.h"

#include <omp.h>

#include "Cubemap.h"
#include "Random.h"

Cubemap *IblBaker::BakeIrradiance(Cubemap *input)
{
	Cubemap *output = new Cubemap(input->GetSize() / 4);
	
	#pragma omp parallel
	{
		auto randomProvider = Random::InCircle(1.0f);

		#pragma omp for collapse(2) 
		for (int face = 0; face < 6; face++)
		{
			for (int i = 0; i < output->GetSize(); i++)
			{
				for (int j = 0; j < output->GetSize(); j++)
				{
					Eigen::Vector2f uv;
					uv.x() = static_cast<float>(i) / output->GetSize();
					uv.y() = static_cast<float>(j) / output->GetSize();

					auto dir = output->GetDirection(static_cast<Cubemap::Face>(face), uv);
					Eigen::Vector3f tempDir = dir + Eigen::Vector3f::Ones();

					Eigen::Matrix3f tangentSpace;
					tangentSpace.col(0) = dir.cross(tempDir).normalized();
					tangentSpace.col(1) = tangentSpace.col(0).cross(dir).normalized();
					tangentSpace.col(2) = dir;

					int sampleCount = 4096 * 16;
					Eigen::Vector4f color = Eigen::Vector4f::Zero();
					for (size_t k = 0; k < sampleCount; k++)
					{
						auto randomVectorInCircle = randomProvider.Pop();
						auto z = std::sqrtf(1 - randomVectorInCircle.squaredNorm());

						Eigen::Vector3f sampleVector;
						sampleVector << randomVectorInCircle, z;
						sampleVector = tangentSpace * sampleVector;
						color += input->Sample(sampleVector) / sampleCount;
					}

					output->PutPixel(static_cast<Cubemap::Face>(face), uv, color);
				}
			}
		}
	}

	return output;
}
