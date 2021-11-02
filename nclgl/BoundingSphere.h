#pragma once
#include "BoundingVolume.h"
class BoundingSphere :
	public BoundingVolume
{
public:
	BoundingSphere(float, Vector3);
	virtual ~BoundingSphere() {};

	int GetRadius() const { return radius; }

protected:
	float radius;
};

