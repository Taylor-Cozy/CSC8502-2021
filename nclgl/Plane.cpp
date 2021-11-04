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
	// OOBB
	BoundingBox bound = *static_cast<BoundingBox*>(n.GetBoundingVolume());
	Vector3 orientedNormal = (Matrix4::Translation(normal) * bound.GetWorldPosition().GetTransposedRotation().Inverse()).GetPositionVector();
	//std::cout << "Normal: " << normal.x << " " << normal.y << " " << normal.z << " | Transposed: " << orientedNormal.x << " " << orientedNormal.y << " " << orientedNormal.z << std::endl;
	Vector3 extents;
	extents = bound.GetBoundingSize();

	if (normal.x < 0)
		extents.x *= -1;
	if (normal.y < 0)
		extents.y *= -1;
	if (normal.z < 0)
		extents.z *= -1;

	extents += bound.GetWorldPosition().GetPositionVector();

	extents = (Matrix4::Translation(extents) * bound.GetWorldPosition().GetTransposedRotation().Inverse()).GetPositionVector();

	float outside = (orientedNormal.x * extents.x) + (orientedNormal.y * extents.y) + (orientedNormal.z * extents.z) + distance;

	if (outside < 0) {
		//std::cout << "Outside!" << std::endl;
		return false;
	}

	return true;
}
