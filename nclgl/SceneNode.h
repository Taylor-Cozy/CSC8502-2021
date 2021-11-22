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
	SceneNode(Mesh* m = NULL, Vector4 colour = Vector4(1, 1, 1, 1), bool shadowExempt = false);
	~SceneNode(void);

	BoundingVolume* GetBoundingVolume() const { return boundingVol; }
	void SetBoundingVolume(BoundingVolume* f) { boundingVol = f; }

	float GetCameraDistance() const { return distanceFromCamera; }
	void SetCameraDistance(float f) { distanceFromCamera = f; }

	GLuint* GetTexture() const { return diffuseTex; }
	void SetTexture(GLuint* tex) { 
		useTexture = 1;
		diffuseTex = tex; }
	void SetBumpMapTexture(GLuint* t) { diffuseBump = t; }
	void SetSecondaryTexture(GLuint* tex, GLuint* bump) {
		useSecond = true;
		secondaryTex = tex;
		secondaryBump = bump;
	}
	void SetThirdTexture(GLuint* tex, GLuint* bump) {
		useThird = true;
		thirdTex = tex; 
		thirdBump = bump;
	}

	void SetLight(Light* l) {
		light = l;
		isLight = true;
	}

	bool IsLight() const { return isLight; }

	void SetShadowTex(GLuint* tex) {
		useShadows = true;
		shadowTex = tex;
	}

	bool CheckShadowExempt() const { return shadowExempt; }

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
	virtual void UpdateVariables(float dt) {
		time += dt;
		if (isLight) {
			transform = transform * Matrix4::Translation(Vector3(0, sin(time) / 100.0f, 0));
			light->SetPosition(transform.GetPositionVector());
		}
	};

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

	float time = 0.0f;

	float distanceFromCamera;
	BoundingVolume* boundingVol;

	int useTexture = 0;
	GLuint* diffuseTex;
	GLuint* diffuseBump;

	bool useSecond = false;
	GLuint* secondaryTex;
	GLuint* secondaryBump;

	bool useThird = false;
	GLuint* thirdTex;
	GLuint* thirdBump;

	bool isLight = false;
	Light* light;

	bool useShadows = false;
	bool shadowExempt;
	GLuint* shadowTex;

	Matrix4 modelMat;	//Model matrix. NOT MODELVIEW
	Matrix4 textureMat;	//Texture matrix
};

