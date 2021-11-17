#include "Renderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/BoundingBox.h"


Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{
	
	quad = Mesh::GenerateQuad();
	debugCube = Mesh::LoadFromMeshFile("cube.msh");
	heightmap = new HeightMap(TEXTUREDIR"noise.png");
	Vector3 heightmapSize = heightmap->GetHeightMapSize();

	root = new SceneNode();

	SceneNode* heightMapNode = new SceneNode(heightmap);
	BoundingBox* b = new BoundingBox(heightmapSize * Vector3(0.5, 0.5, 0.5), heightMapNode->GetWorldTransform());
	b->SetOffset(heightmapSize * 0.5);
	heightMapNode->SetBoundingVolume(b);
	
	root->AddChild(heightMapNode);

	WaterNode* water = new WaterNode(heightmap->GetHeightMapSize(), quad);
	water->SetTransform(
		Matrix4::Translation(heightmapSize * 0.5f) *
		Matrix4::Scale(heightmapSize * 0.5f) *
		Matrix4::Rotation(90, Vector3(1, 0, 0))
	);
	BoundingBox* waterBound = new BoundingBox(heightmapSize, water->GetWorldTransform());
	water->SetBoundingVolume(waterBound);

	heightMapNode->AddChild(water);

#pragma region Init Textures
	waterTexture = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	waterBump = SOIL_load_OGL_texture(TEXTUREDIR"waterbump.PNG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTexture = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	heightmapTex = SOIL_load_OGL_texture(TEXTUREDIR"noise.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg",
		TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg",
		TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!waterTexture || !waterBump || !earthTexture || !earthBump || !cubeMap || !heightmapTex)
		return;

	SetTextureRepeating(waterTexture, true);
	SetTextureRepeating(waterBump, true);
	SetTextureRepeating(earthTexture, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(heightmapTex, true);
#pragma endregion

	heightMapNode->SetTexture(&earthTexture);
	water->SetTexture(&waterTexture);
	water->SetBumpMapTexture(&waterBump);
	water->SetCubeMapTexture(&cubeMap);
	water->SetHeightMapTexture(&heightmapTex);

#pragma region Init Shaders
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
	defaultShader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");

	if (!lightShader->LoadSuccess() || !defaultShader->LoadSuccess() ||
		!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess())
		return;
#pragma endregion

	heightMapNode->SetShader(defaultShader);
	water->SetShader(reflectShader);

	camera = new Camera(-45.0f, 0.0f, 0.0f, heightmapSize * Vector3(0.5f, 5.0f, 0.5f));

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	init = true;
}
Renderer::~Renderer(void)	{
	delete root;
	delete camera;
	delete heightmap;
	delete lightShader;
	delete defaultShader;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	waterRotate += dt * .20f;
	waterCycle += dt * 0.025f;

	root->Update(dt);
}
// TODO reflect shader
void Renderer::BuildNodeLists(SceneNode* from) {
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists(*i);
	}
}

void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.rbegin(), transparentNodeList.rend(), SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::DrawNodes() {

	BindShader(defaultShader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(defaultShader->GetProgram(), "diffuseTex"), 0);
	//glEnable(GL_DEPTH_TEST);

	for (const auto& i : nodeList) {
		DrawNode(i);
	}

	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
}

void Renderer::DrawDebugNodes() {

	BindShader(defaultShader);
	UpdateShaderMatrices();

	//glDisable(GL_DEPTH_TEST);

	for (const auto& i : nodeList) {
		DrawDebugNode(i);
	}

	for (const auto& i : transparentNodeList) {
		DrawDebugNode(i);
	}
}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		Shader* currentShader = n->GetShader();
		BindShader(currentShader);
		glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
		n->SetShaderVariables();
		modelMatrix = n->GetModelMat();
		textureMatrix = n->GetTextureMat();
		UpdateShaderMatrices();

		n->Draw(*this);
	}
}

void Renderer::DrawDebugNode(SceneNode* n) {
	if (n->GetMesh()) {
		Matrix4 model = n->GetBoundingVolume()->GetWorldPosition() * Matrix4::Scale(Vector3(2,2,2));
		glUniformMatrix4fv(glGetUniformLocation(defaultShader->GetProgram(), "modelMatrix"), 1, false, (model).values);
		glUniform4fv(glGetUniformLocation(defaultShader->GetProgram(), "nodeColour"), 1, (float*)& Vector4(0.98, 0.28, 0.76, 0.5));
		debugCube->Draw();
		//debugSphere->Draw();
	}
}

void Renderer::RenderScene() {
	BuildNodeLists(root);
	SortNodeLists();

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	DrawNodes();

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	DrawSkyBox();

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	//glDisable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//DrawDebugNodes();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glEnable(GL_DEPTH_TEST);
	ClearNodeLists();
}

void Renderer::DrawSkyBox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}