#pragma once
#include "SceneNode.h"
class WaterNode : public SceneNode
{
public:
	WaterNode(Mesh* m = NULL, Vector4 colour = Vector4(1, 1, 1, 1));
	~WaterNode(void);
	
	void Update(float dt) override;
	void Draw(const OGLRenderer& r) override;

protected:
	float waterRotate = 0.0f;
	float waterCycle = 0.0f;

	float time = 0.0f;

	GLuint* cubeTex;
	GLuint* bumpTex;
	GLuint* heightMapTex;

};

