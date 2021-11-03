#pragma once
#include "Vector3.h"
#include "SceneNode.h"
#include "BoundingSphere.h"
#include "BoundingBox.h"

class Plane
{
public:
	Plane(void) {};
	Plane(const Vector3& normal, float distance, bool normalise = false);
	~Plane(void) {};

	void SetNormal(const Vector3& normal) { this->normal = normal; }
	Vector3 GetNormal() const { return normal; }

	void SetDistance(float dist) { distance = dist; }
	float GetDistance() const { return distance; }

	bool SphereInPlane(const Vector3& position, float radius) const;
	bool CheckInPlane(SceneNode& n);
	bool SphereInPlane(SceneNode& n);
	bool BoxInPlane(SceneNode& n);
	
protected:
	Vector3 normal;
	float distance;
};

