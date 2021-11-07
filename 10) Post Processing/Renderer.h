#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Camera.h"

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void PresentScene();
	void DrawPostProcess(GLuint* textureArray, Shader* processShader);
	void DrawScene();
	void DrawMap();

	Shader* sceneShader;
	Shader* processShader;
	Shader* mapShader;

	Camera* camera;
	Camera* mapView;

	Matrix4 mapViewMatrix;
	Matrix4 mapProjMatrix;
	Matrix4 sceneViewMatrix;

	Mesh* quad;
	Mesh* circle;
	HeightMap* heightMap;
	GLuint heightTexture;
	GLuint bufferFBO;
	GLuint mapFBO;
	GLuint processFBO;
	GLuint bufferColourTex[2];
	GLuint mapColourTex[2];
	GLuint bufferDepthTex;
	GLuint mapDepthTex;

	int screenWidth, screenHeight;
};

