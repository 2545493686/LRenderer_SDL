#include "IblBaker.h"

#include <iostream>
#include <omp.h>

#include "Cubemap.h"
#include "Random.h"
#include "MathUtils.h"

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
					uv.x() = static_cast<float>(j) / output->GetSize();
					uv.y() = static_cast<float>(i) / output->GetSize();

					auto dir = output->GetDirection(static_cast<Cubemap::Face>(face), uv);
					Eigen::Matrix3f tangentSpace;
					Eigen::Vector3f up = dir + vec3(1);
					Eigen::Vector3f tangent = dir.cross(up).normalized();
					Eigen::Vector3f bitangent = dir.cross(tangent).normalized();
					
					tangentSpace.col(0) = tangent;
					tangentSpace.col(1) = bitangent;
					tangentSpace.col(2) = dir;

					int sampleCount = 4096;
					Eigen::Vector4d color = Eigen::Vector4d::Zero();
					int realSampleCount = 0;
					for (size_t k = 0; k < sampleCount; k++)
					{
						auto randomVectorInCircle = randomProvider.Pop();
						auto z = std::sqrtf(1 - randomVectorInCircle.squaredNorm());

						Eigen::Vector3f sampleVector;
						sampleVector << randomVectorInCircle, z;
						sampleVector = sampleVector.normalized();
						sampleVector = tangentSpace * sampleVector;
						
						Eigen::Vector4d sampleColor = input->Sample(sampleVector).cast<double>();
						MathUtils::ClampVector4(sampleColor, 0.0, 10.0);

						color = sampleColor / static_cast<double>(realSampleCount + 1)
							+ color * static_cast<double>(realSampleCount) / static_cast<double>(realSampleCount + 1);
						
						realSampleCount++;
						assert(MathUtils::IsNan(color));
					}


					output->PutPixel(static_cast<Cubemap::Face>(face), j, i, color.cast<float>());
				}
			}
		}
	}

	return output;
}
