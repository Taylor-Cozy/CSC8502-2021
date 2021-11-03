#include "Plane.h"

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
	if (Vector3::Dot(position, normal) + distance <= -radius) {
		return false;
	}
	return true;
}

bool Plane::CheckInPlane(SceneNode& n)
{
	if (n.GetBoundingVolume()->GetType() == BOX)
		return BoxInPlane(n);
	
	if (n.GetBoundingVolume()->GetType() == SPHERE)
		return SphereInPlane(n);

	return false;
}

bool Plane::SphereInPlane(SceneNode& n)
{
	BoundingSphere* bound = static_cast<BoundingSphere*>(n.GetBoundingVolume());

	if (Vector3::Dot(bound->GetWorldPosition().GetPositionVector(), normal) + distance <= -bound->GetBoundingRadius()) {
		return false;
	}
	return true;
}

bool Plane::BoxInPlane(SceneNode& n)
{
	BoundingBox* bound = static_cast<BoundingBox*>(n.GetBoundingVolume());
	float maxSize = std::max(std::max(abs(bound->GetBoundingSize().x), abs(bound->GetBoundingSize().y)), abs(bound->GetBoundingSize().z));

	if (Vector3::Dot(bound->GetWorldPosition().GetPositionVector(), normal) + distance <= -maxSize) {
		return false;
	}
	return true;
}
