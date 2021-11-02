#include "Plane.h"
#include <math.h>
Plane::Plane(const Vector3& normal, float distance, bool normalise) {
	if (normalise) {
		float length = sqrt(Vector3::Dot(normal, normal));

		this->normal = normal / length;
		this->distance = distance / length;
	}
	else {
		this->normal = normal;
		this->distance = distance;
	}
}

bool Plane::SphereInPlane(const Vector3& position, float radius) const {
	if (Vector3::Dot(position, normal) + distance <= 0 - radius) {
		return false;
	}
	return true;
}

bool Plane::CheckCollision(const BoundingVolume* node) const
{
	if(node->GetType() == RECTANGLE)
		return CheckCollision(static_cast<const BoundingRectangle*>(node));
	if (node->GetType() == SPHERE)
		return CheckCollision(static_cast<const BoundingSphere*>(node));
	return false;
}

bool Plane::CheckCollision(const BoundingSphere* node) const
{
	// (N.V) + d < 0 then point outside of plane, accounting for radius
	if (Vector3::Dot(node->GetPosition(), normal) + distance <= 0 - node->GetRadius()) {
		return false;
	}
	return true;
}


bool Plane::CheckCollision(const BoundingRectangle* node) const
{
	Vector3 vec1;
	//if(normal.x > 0)
		//vec1.x = node->GetPosition().x - (node->GetWidth() / 2)

	float point = Vector3::Dot(node->GetPosition(), normal) + distance;
	float maxExtent = std::max(std::max(node->GetWidth(), node->GetHeight()), node->GetDepth());
	// (N.V) + d < 0 then point outside of plane, accounting for radius
	if (point <= 0 - maxExtent) {
		return false;
	}
	return true;
}