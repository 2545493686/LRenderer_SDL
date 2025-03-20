#include "IblBaker.h"

#include "Cubemap.h"

Cubemap *IblBaker::BakeIrradiance(Cubemap *input)
{
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
				auto color = input->Sample(dir);
				
				output->PutPixel(static_cast<Cubemap::Face>(face), uv, color);
			}
		}
	}

	return output;
}
