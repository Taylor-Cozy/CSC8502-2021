#pragma once

#include "Vector4.h"
#include "Vector3.h"
#include "Mesh.h"

enum lightType {
	POINT_LIGHT,
	DIRECTIONAL_LIGHT
};

class Light
{
public:
	Light() {}
	Light(const Vector3& position, const Vector4& colour, lightType type, bool castShadows = false) :
		position(position), colour(colour), specColour(colour), type(type), castShadows(castShadows) {
	}

	Light(const Vector3& position, const Vector4& colour, const Vector4& specColour, lightType type, bool castShadows = false) :
		position(position), colour(colour), specColour(specColour), type(type), castShadows(castShadows) {
	}

	~Light(void) {
	};

	Vector3 GetPosition() const { return position; }
	void SetPosition(const Vector3& val) { position = val; }

	Vector4 GetColour() const { return colour; }
	void SetColour(const Vector4& val) { colour = val; }

	Vector4 GetSpecColour() const { return specColour; }
	void SetSpecColour(const Vector4& val) { specColour = val; }

	int GetType() const { return type; }
	void SetType(lightType l) { type = l; }

	void SetShadowMatrix(Matrix4 shadMat) { shadowMatrix = shadMat;	}
	Matrix4 GetShadowMatrix() const { return shadowMatrix; }

	bool CheckCastShadows() const { return castShadows; }

protected:
	void CreateFBO();

	Vector3 position;
	Vector4 colour;
	Vector4 specColour;
	lightType type;
	Matrix4 shadowMatrix;
	bool castShadows = false;
};

