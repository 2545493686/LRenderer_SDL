#pragma once
#include "TextureOperator.h"

class HoughOperator : public TextureOperator
{
public:
	// input
	Texture *dx;
	Texture *dy;

	// output
	Texture *circlePad;

	~HoughOperator()
	{
		if (circlePad)
		{
			delete circlePad;
		}
	}

	std::vector<Eigen::Vector2i> GetLinePoints(Eigen::Vector2f begin, Eigen::Vector2f dir, float maxR);


	// Í¨¹ý TextureOperator ¼Ì³Ð
	void Invoke(Texture *tex) override;

private:
	void DDALine(Eigen::Vector2f start, Eigen::Vector2f end, std::vector<Eigen::Vector2i> &points, bool includeStart);
	void DrawPoints(Texture *tex, std::vector<Eigen::Vector2i> points);
	void DrawCircle(Texture *tex, int maxIndexX, int maxIndexY, int maxIndexR);
};

