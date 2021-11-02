#include "BoundingRectangle.h"

BoundingRectangle::BoundingRectangle(float width, float height, float depth , Vector3 position) : 
	BoundingVolume(RECTANGLE, position), width(width), height(height), depth(depth)
{
}

BoundingRectangle::BoundingRectangle(Vector3 scale, Vector3 position) :
	BoundingVolume(RECTANGLE, position), width(scale.x), height(scale.y), depth(scale.z)
{
}