#include "IblBaker.h"

#include "Cubemap.h"
#include "Random.h"

Cubemap *IblBaker::BakeIrradiance(Cubemap *input)
{
	static auto randomProvider = Random::InCircle(1.0f);
	Cubemap *output = new Cubemap(input->GetSize() / 4);
	
	for (size_t face = 0; face < 6; face++)
	{
		for (size_t i = 0; i < output->GetSize(); i++)
		{
			for (size_t j = 0; j < output->GetSize(); j++)
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

				int sampleCount = 64;
				Eigen::Vector4f color = Eigen::Vector4f::Zero();
				for (size_t i = 0; i < sampleCount; i++)
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

	return output;
}
