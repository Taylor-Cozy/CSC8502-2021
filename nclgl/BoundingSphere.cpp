#include "BoundingSphere.h"

BoundingSphere::BoundingSphere(float radius, Vector3 position) : BoundingVolume(SPHERE, position), radius(radius)
{
}
