#pragma once

#include "Vector4.h"
#include "Vector3.h"
#include "Mesh.h"

class Light
{
public:
	Light() {}
	Light(const Vector3& position, const Vector4& colour, float radius) : 
		position(position), colour(colour), radius(radius), specColour(colour) {}

	Light(const Vector3& position, const Vector4& colour, const Vector4& specColour, float radius) :
		position(position), colour(colour), radius(radius), specColour(specColour) {}

	~Light(void) {};

	Vector3 GetPosition() const { return position; }
	void SetPosition(const Vector3& val) { position = val; }

	float GetRadius() const { return radius; }
	void SetRadius(float val) { radius = val; }

	Vector4 GetColour() const { return colour; }
	void SetColour(const Vector4& val) { colour = val; }

	Vector4 GetSpecColour() const { return specColour; }
	void SetSpecColour(const Vector4& val) { specColour = val; }

protected:
	Vector3 position;
	float radius;
	Vector4 colour;
	Vector4 specColour;
};

