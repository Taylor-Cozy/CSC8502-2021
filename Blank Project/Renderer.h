#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"

class Camera;
class SceneNode;
class Mesh;
class Shader;
class HeightMap;

class Renderer : public OGLRenderer	{
public:
	Renderer(Window &parent);
	~Renderer(void);

	void RenderScene()				override;
	void UpdateScene(float msec)	override;

protected:
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();

	void DrawNodes();
	void DrawNode(SceneNode* n);

	void DrawDebugNodes();
	void DrawDebugNode(SceneNode* n);

	void DrawSkyBox();

	float waterRotate = 0.0f;
	float waterCycle = 0.0f;

	SceneNode* root;
	Camera* camera;
	HeightMap* heightmap;

	Shader* lightShader;
	Shader* defaultShader;
	Shader* reflectShader;
	Shader* skyboxShader;

	Mesh* debugCube;
	Mesh* cube;
	Mesh* sphere;
	Mesh* quad;
	Mesh* circle;

	GLuint cubeMap;
	GLuint earthTexture;
	GLuint earthBump;
	GLuint waterTexture;
	GLuint waterBump;

	Frustum frameFrustum;

	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
};
