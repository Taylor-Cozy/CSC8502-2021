#include "BoundingBox.h"

BoundingBox::BoundingBox(Vector3 boundingSize, Matrix4 worldPos) : BoundingVolume(worldPos, BOX), boundingSize(boundingSize)
{
}
