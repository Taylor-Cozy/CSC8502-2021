#pragma once
#include "Vector3.h"
#include "Mesh.h"

enum type {
	SPHERE,
	RECTANGLE
};

class BoundingVolume
{
public:
	BoundingVolume(type, Vector3);
	virtual ~BoundingVolume() {};

	type GetType() const { return volumeType; }
	Vector3 GetPosition() const { return worldPos.GetPositionVector(); }
	Matrix4 GetWorldPos() const { return worldPos; }
	void SetPosition(Vector3 newPos) { position = newPos; }
	void SetWorldPos(Matrix4 newPos) { worldPos = newPos; }

protected:
	type volumeType;
	Vector3 position;
	Matrix4 worldPos;
	Matrix4 modelMat;
};

