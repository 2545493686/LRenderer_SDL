#include "MipmapBaker.h"

#include "Eigen/Dense"
#include "Cubemap.h"
#include "Random.h"
#include "MathUtils.h"

std::vector<Cubemap *> MipmapBaker::BakeMipmap(Cubemap *basis)
{
	std::vector<Cubemap *> mipmaps = std::vector<Cubemap *>();
	mipmaps.push_back(basis);

	while (mipmaps.back()->GetSize() > 1)
	{
		Cubemap *input = mipmaps.back();
		Cubemap *output = new Cubemap(mipmaps.back()->GetSize() / 2);
		mipmaps.push_back(output);

		#pragma omp parallel
		{
			#pragma omp for collapse(2)
			for (int face = 0; face < 6; face++)
			{
				for (int i = 0; i < output->GetSize(); i++)
				{
					for (int j = 0; j < output->GetSize(); j++)
					{
						Eigen::Vector4d color = Eigen::Vector4d::Zero();
						color += input->SampleFace(static_cast<Cubemap::Face>(face), j * 2, i * 2).cast<double>();
						color += input->SampleFace(static_cast<Cubemap::Face>(face), j * 2, i * 2 + 1).cast<double>();
						color += input->SampleFace(static_cast<Cubemap::Face>(face), j * 2 + 1, i * 2).cast<double>();
						color += input->SampleFace(static_cast<Cubemap::Face>(face), j * 2 + 1, i * 2 + 1).cast<double>();
						color /= 4.0;

						output->PutPixel(static_cast<Cubemap::Face>(face), j, i, color.cast<float>());
					}
				}
			}
		}
	}

	return mipmaps;
}
