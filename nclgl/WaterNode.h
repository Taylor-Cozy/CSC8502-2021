#pragma once
#include "SceneNode.h"
#include "HeightMap.h"

class WaterNode : public SceneNode
{
public:
	WaterNode(Vector3 hSize, float* sunTime, Mesh* m = NULL, Vector4 colour = Vector4(1, 1, 1, 0.75));
	~WaterNode(void);
	
	void UpdateVariables(float dt) override;
	void SetShaderVariables() override;

	void SetCubeMapTexture(GLuint* t) { cubeTex = t; }
	void SetMtnCubeMapTexture(GLuint* t) { mtnTex = t; }
	void SetHeightMapTexture(GLuint* t) { heightMapTex = t; }

protected:
	float waterRotate = 0.0f;
	float waterCycle = 0.0f;

	float time = 0.0f;
	float* sunTime;

	GLuint* cubeTex;
	GLuint* mtnTex;
	GLuint* heightMapTex;
	
	Vector3 hSize;
};

