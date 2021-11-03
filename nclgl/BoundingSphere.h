#pragma once
#include "Matrix4.h"
#include "BoundingVolume.h"

class BoundingSphere : public BoundingVolume
{
public:
	BoundingSphere(float, Matrix4);
	virtual ~BoundingSphere() {};

	float GetBoundingRadius() const { return boundingRadius; }
	void SetBoundingRadius(float f) { boundingRadius = f; }

protected:
	float boundingRadius;
};

