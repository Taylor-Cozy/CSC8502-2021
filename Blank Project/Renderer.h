#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "../nclgl/WaterNode.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"

class Camera;
class SceneNode;
class Mesh;
class Shader;
class HeightMap;
class Light;

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

	void DrawNodes(bool shadowPass = false);
	void DrawNode(SceneNode* n, bool shadowPass);

	void DrawDebugNodes();
	void DrawDebugNode(SceneNode* n);

	void DrawSkyBox();

	void FillBuffers();
	void DrawPointLights();
	void CombineBuffers();

	void GenerateScreenTexture(GLuint& into, bool depth = false);

	void DrawShadowScene();
	void DrawPointShadowScene();
	void DrawMainScene();

	void PresentScene();
	void DrawPostProcess(GLuint* textureArray, Shader* processShader, int numberPasses = 1);

	SceneNode* root;

	Camera* camera;
	Camera* mapView;

	float time = 0.0f;
	HeightMap* heightmap;
	Mesh* debugCube;
	Mesh* cube;
	Mesh* sphere;
	Mesh* quad;
	Mesh* circle;

	Shader* lightShader;
	Shader* defaultShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* sceneShader;
	Shader* combineShader;
	Shader* shadowShader;
	Shader* processShader;
	Shader* mapProcessShader;
	Shader* mapSceneShader;

	GLuint cubeMap;
	GLuint earthTexture;
	GLuint earthBump;
	GLuint waterTexture;
	GLuint waterBump;
	GLuint sandTexture;
	GLuint sandBump;
	GLuint heightmapTex;
	GLuint rockTexture;
	GLuint rockBump;

	GLuint shadowFBO;
	GLuint shadowTex;
	GLuint bufferFBO;
	GLuint mapFBO;
	GLuint processFBO;

	GLuint bufferColourTex[2];
	GLuint mapColourTex[2];
	GLuint bufferDepthTex;
	GLuint mapDepthTex;

	GLuint shadowCubeFBO;
	GLuint shadowCubeTex;

	Frustum frameFrustum;

	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
	vector<SceneNode*> lightNodeList;
	vector<Light*> lights;
	Light* pointLight;
	Light* pointLight2;

	Matrix4 rotation[6];
	Matrix4 mapViewMatrix;
	Matrix4 mapProjMatrix;
	Matrix4 sceneViewMatrix;
};
