#pragma once
#include "Matrix4.h"
#include "Vector3.h"

enum type {
	SPHERE,
	BOX
};

class BoundingVolume
{
public:

	BoundingVolume(Matrix4, type);
	virtual ~BoundingVolume() {};

	Matrix4 GetWorldPosition() const { return worldPos; }
	void SetWorldPosition(Matrix4 wP) { worldPos = wP; }

	Vector3 GetOffset() const { return offset; }
	void SetOffset(Vector3 off) { offset = off; }

	type GetType() const { return shape; }
	void SetType(type t) { shape = t; }

protected:
	Matrix4 worldPos;
	Vector3 offset;
	type shape;
};

