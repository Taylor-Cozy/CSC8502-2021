#pragma once
#include "BoundingVolume.h"
class BoundingRectangle :
	public BoundingVolume
{
public:
	BoundingRectangle(float, float, float , Vector3);
	BoundingRectangle(Vector3, Vector3);
	virtual ~BoundingRectangle() {}

	float GetWidth() const { return width; }
	float GetHeight() const { return height; }
	float GetDepth() const { return depth; }

protected:
	float width, height, depth;
};

