#pragma once
#include "../nclgl/OGLRenderer.h"

class Camera;
class Shader;
class HeightMap;

const int MAX_LIGHTS = 64;

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void DrawHeightMap();
	void DrawWater();
	void DrawSkyBox();
	void DrawMirror();

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* mirrorShader;

	HeightMap* heightMap;
	Mesh* quad;
	Mesh* circle;

	vector<Light*> lights;
	Camera* camera;

	GLuint cubeMap;
	GLuint waterTex;
	GLuint waterBump;
	GLuint earthTex;
	GLuint earthBump;
	GLuint heightmapTex;

	float time;

	float waterRotate;
	float waterCycle;
};

