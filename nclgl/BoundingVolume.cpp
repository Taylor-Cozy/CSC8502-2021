#include "BoundingVolume.h"

BoundingVolume::BoundingVolume(Matrix4 worldPos, type shape) : worldPos(worldPos), shape(shape)
{
	offset = Vector3(0, 0, 0);
}
