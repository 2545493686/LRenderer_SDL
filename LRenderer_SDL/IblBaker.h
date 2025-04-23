#pragma once  
#include "Eigen/Dense"  
#include "Random.h"  
#include "MathUtils.h"

class Cubemap;  
class LatitudeLongitudeMap;  

class IblBaker  
{  
   IblBaker() = delete;  

public:  
   static Cubemap * BakeIrradiance(Cubemap *input, int size);
   static LatitudeLongitudeMap * BakeIrradiance(LatitudeLongitudeMap *input, int sampleCount = 16384);
   static Cubemap * BakeRadiance(Cubemap *input, float roughness, int inverseScale);  

private:  
   using SampleFunc = std::function<Eigen::Vector4f(const Eigen::Vector3f &)>;
   
   template<typename T> requires std::convertible_to<T, SampleFunc> EIGEN_ALWAYS_INLINE
   static void SampleIrradiance(Eigen::Vector3f &dir, Eigen::Vector4d &outputColor, int sampleCount, CircleRandomProvider &randomProvider, T sampleFunc)
   {
	   //outputColor = sampleFunc(dir).cast<double>();
	   //return;

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

	   outputColor = Eigen::Vector4d::Zero();
	   int realSampleCount = 0;
	   for (size_t k = 0; k < sampleCount; k++)
	   {
		   auto randomVectorInCircle = randomProvider.Pop();
		   auto z = std::sqrtf(1 - randomVectorInCircle.squaredNorm());

		   Eigen::Vector3f sampleVector;
		   sampleVector << randomVectorInCircle, z;
		   sampleVector = sampleVector.normalized();
		   sampleVector = tangentSpace * sampleVector;

		   Eigen::Vector4d sampleColor = sampleFunc(sampleVector).cast<double>();
		   MathUtils::ClampVector4(sampleColor, 0.0, 1000.0);

		   outputColor = sampleColor / static_cast<double>(realSampleCount + 1)
			   + outputColor * static_cast<double>(realSampleCount) / static_cast<double>(realSampleCount + 1);  // 已经包含brdf项（1/pi）

		   realSampleCount++;

		   assert(!MathUtils::IsNan(outputColor));
	   }
   }
};
