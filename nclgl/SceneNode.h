#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Mesh.h"
#include "BoundingVolume.h"

#include <vector>

class SceneNode
{
public:
	SceneNode(Mesh* m = NULL, Vector4 colour = Vector4(1, 1, 1, 1));
	~SceneNode(void);

	BoundingVolume* GetBoundingVolume() const { return boundingVol; }
	void SetBoundingVolume(BoundingVolume* f) { boundingVol = f; }

	float GetCameraDistance() const { return distanceFromCamera; }
	void SetCameraDistance(float f) { distanceFromCamera = f; }

	GLuint* GetTexture() const { return texture; }
	void SetTexture(GLuint* tex) { texture = tex; }

	static bool CompareByCameraDistance(SceneNode* a, SceneNode* b) {
		return (a->distanceFromCamera < b->distanceFromCamera) ? true : false;
	}

	void SetTransform(const Matrix4& matrix) { transform = matrix; }
	const Matrix4& GetTransform() const { return transform; }
	Matrix4 GetWorldTransform() const { return worldTransform; }

	Vector4 GetColour() const { return colour; }
	void SetColour(Vector4 c) { colour = c; }

	Vector3 GetModelScale() const { return modelScale; }
	void SetModelScale(Vector3 s) { modelScale = s; }

	Mesh* GetMesh() const { return mesh; }
	void SetMesh(Mesh* m) { mesh = m; }

	Shader* GetShader() const { return shader; }
	void SetShader(Shader* sh) { shader = sh; }

	virtual void SetShaderVariables();

	Matrix4 GetModelMat() { return modelMat; }
	Matrix4 GetTextureMat() { return textureMat; }

	void AddChild(SceneNode* s);
	void RemoveChild(SceneNode* s);

	virtual void Update(float dt);
	virtual void Draw(const OGLRenderer& r);
	virtual void UpdateVariables(float dt) {};

	std::vector<SceneNode*>::const_iterator GetChildIteratorStart() { return children.begin(); }
	std::vector<SceneNode*>::const_iterator GetChildIteratorEnd() { return children.end(); }

protected:
	SceneNode* parent;
	Mesh* mesh;
	Shader* shader;
	Matrix4 worldTransform;
	Matrix4 transform;
	Vector3 modelScale;
	Vector4 colour;
	std::vector<SceneNode*> children;

	float distanceFromCamera;
	BoundingVolume* boundingVol;
	GLuint* texture;

	Matrix4 modelMat;	//Model matrix. NOT MODELVIEW
	Matrix4 textureMat;	//Texture matrix
};

