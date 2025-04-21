#include "IblBaker.h"

#include <iostream>
#include <omp.h>
#include <cmath>

#include "Cubemap.h"
#include "Random.h"
#include "MathUtils.h"
#include "LatitudeLongitudeMap.h"


Cubemap *IblBaker::BakeIrradiance(Cubemap *input, int size)
{
	Cubemap *output = new Cubemap(size);
	
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
					uv.x() = (static_cast<float>(j) + 0.5f) / output->GetSize();
					uv.y() = (static_cast<float>(i) + 0.5f) / output->GetSize();

					auto dir = output->GetDirection(static_cast<Cubemap::Face>(face), uv);
					
					Eigen::Vector4d color;
					int sampleCount = 4;

					SampleIrradiance(dir, color, sampleCount, randomProvider,  [&input](const Eigen::Vector3f &vec)
					{
						return input->Sample(vec);
					});

					output->PutPixel(static_cast<Cubemap::Face>(face), j, i, color.cast<float>());
				}
			}
		}
	}

	return output;
}

LatitudeLongitudeMap *IblBaker::BakeIrradiance(LatitudeLongitudeMap *input)
{
	LatitudeLongitudeMap *output = new LatitudeLongitudeMap(input->width, input->height);

#pragma omp parallel
	{
		auto randomProvider = Random::InCircle(0.01f);

#pragma omp for collapse(1) 
		for (int i = 0; i < output->height; i++)
		{
			for (int j = 0; j < output->width; j++)
			{
				Eigen::Vector2f uv;
				uv.x() = (static_cast<float>(j) + 0.5f) / output->width;
				uv.y() = (static_cast<float>(i) + 0.5f) / output->height;

				auto dir = output->GetDirection(uv);

				Eigen::Vector4d color;
				int sampleCount = 4;

				SampleIrradiance(dir, color, sampleCount, randomProvider, [&input](const Eigen::Vector3f &vec)
				{
					return input->Sample(vec);
				});

				output->PutPixel(j, i, color.cast<float>());
			}
		}
	}

	return output;
}

// https://zhuanlan.zhihu.com/p/34944420
Eigen::Vector3f ImportanceSampleGGX(Eigen::Vector2f E, float a2)
{
	float Phi = 2 * M_PI * E.x();
	float CosTheta = sqrt((1 - E.y()) / (1 + (a2 - 1) * E.y()));
	if (isnan(CosTheta))
	{
		CosTheta = 1;
	}
	float SinTheta = sqrt(1 - CosTheta * CosTheta);
	if (isnan(SinTheta))
	{
		SinTheta = 0;
	}

	Eigen::Vector3f H;
	H.x() = SinTheta * cos(Phi);
	H.y() = SinTheta * sin(Phi);
	H.z() = CosTheta;

	return H;
}

Cubemap *IblBaker::BakeRadiance(Cubemap *input, float roughness, int inverseScale)
{
	Cubemap *output = new Cubemap(input->GetSize() / inverseScale);

#pragma omp parallel
	{
		auto randomProvider = Random::InRange(0.0f, 1.0f);

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
					Eigen::Vector3f up = Eigen::Vector3f(0, 1, 0);
					if ((up - dir).squaredNorm() < 0.01f)
					{
						up = Eigen::Vector3f(0, -1, 0);
					}
					Eigen::Vector3f tangent = dir.cross(up).normalized();
					Eigen::Vector3f bitangent = dir.cross(tangent).normalized();

					tangentSpace.col(0) = tangent;
					tangentSpace.col(1) = bitangent;
					tangentSpace.col(2) = dir;

					int sampleCount = 4096 * 32 * roughness;
					//int sampleCount = 128;
					Eigen::Vector4d color = Eigen::Vector4d::Zero();
					double normalizeSum = 0;
					for (size_t k = 0; k < sampleCount; k++)
					{
						Eigen::Vector2f E;
						E << randomProvider.Pop(), randomProvider.Pop();
						Eigen::Vector3f sampleVector = ImportanceSampleGGX(E, roughness * roughness);
						sampleVector = sampleVector.normalized();
						sampleVector = tangentSpace * sampleVector;

						Eigen::Vector4d sampleColor = input->Sample(sampleVector).cast<double>();
						MathUtils::ClampVector4(sampleColor, 0.0, 1000.0);

						float sDotD = std::max(sampleVector.dot(dir), 0.0f);
						normalizeSum += static_cast<double>(sDotD);

						color += sampleColor * sDotD;
						assert(MathUtils::IsNan(color));
					}

					color /= normalizeSum;
					output->PutPixel(static_cast<Cubemap::Face>(face), j, i, color.cast<float>());
				}
			}
		}
	}

	return output;
}
