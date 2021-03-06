#pragma once

#include "..//nclgl/OGLRenderer.h"
#include "..//nclgl/SceneNode.h"
#include "..//nclgl/Frustum.h"

class Camera;
class SceneNode;
class Mesh;
class Shader;

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void UpdateScene(float msec) override;
	void RenderScene() override;

protected:
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawDebugNodes();
	void DrawNode(SceneNode* n);
	void DrawDebugNode(SceneNode* n);

	SceneNode* root;
	Camera* camera;
	Mesh* quad;
	Mesh* cube;
	Mesh* debugSphere;
	Shader* shader;
	Shader* debugShader;
	GLuint texture;

	Frustum frameFrustum;

	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
};

