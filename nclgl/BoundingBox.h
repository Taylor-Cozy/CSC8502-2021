#pragma once
#include "Matrix4.h"
#include "BoundingVolume.h"


class BoundingBox : public BoundingVolume
{
public:
	BoundingBox(Vector3, Matrix4);
	virtual ~BoundingBox() {};

	Vector3 GetBoundingSize() const { return boundingSize; }
	void SetBoundingSize(Vector3 f) { boundingSize = f; }

protected:
	Vector3 boundingSize;
};

