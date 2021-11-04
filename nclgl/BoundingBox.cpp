#include "BoundingBox.h"

BoundingBox::BoundingBox(Vector3 boundingSize, Matrix4 worldPos) : BoundingVolume(worldPos, BOX)
{
	this->boundingSize = Vector3(abs(boundingSize.x), abs(boundingSize.y), abs(boundingSize.z));
}
