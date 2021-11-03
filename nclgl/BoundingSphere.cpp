#include "BoundingSphere.h"

BoundingSphere::BoundingSphere(float boundingRadius, Matrix4 worldPos) : BoundingVolume(worldPos, SPHERE), boundingRadius(boundingRadius)
{
}
