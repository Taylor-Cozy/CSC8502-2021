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
class MeshAnimation;
class MeshMaterial;

class Renderer : public OGLRenderer	{
public:
	Renderer(Window &parent);
	~Renderer(void);

	void RenderScene()				override;
	void UpdateScene(float msec)	override;
	void ToggleTrack(bool toggle) {
		if (toggle) {
			followTrack = false;
		}
		else {
			curTime = 0.0f;
			followTrack = true;
			currentWaypoint = 0;
		}
	}

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
	void DrawBloom();
	void CombineBloom();

	void DrawSoldier();

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
	Mesh* soldier;
	MeshAnimation* soldierAnim;
	MeshMaterial* soldierMaterial;
	vector<GLuint> soldierTextures;
	int currentFrame;
	float frameTime;

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
	Shader* bloomShader;
	Shader* combineBloomShader;
	Shader* skinningShader;

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
	GLuint bloomFBO;
	
	GLuint bloomColourTex[3];
	GLuint bufferColourTex[2];
	GLuint mapColourTex[2];
	GLuint bufferDepthTex;
	GLuint bloomDepthTex;
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

	vector<Vector3> waypoints;
	vector<Vector2> waypointRot;
	vector<float> waypointTimes;
	Vector3 oldPos;
	Vector2 oldRot;
	int currentWaypoint = 0;
	float distance(Vector3 a, Vector3 b) {
		return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
	}
	Vector3 lerp(Vector3 a, Vector3 b, float t) {
		return (a * t) + (b * (1.0f - t));
	}
	float lerp(float a, float b, float t) {
		return (a * t) + (b * (1.0f - t));
	}
	bool followTrack = true;
	float waypointTime = 5.0f;
	float curTime = 0.0f;
	bool wait = false;
	float curWaitTime = 0.0f;
};
