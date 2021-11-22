#include "Renderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/BoundingBox.h"
#include "../nclgl/WaterNode.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Light.h"

#define SHADOWSIZE 4096

Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{
	
	quad = Mesh::GenerateQuad();
	circle = Mesh::GenerateCircle();
	debugCube = Mesh::LoadFromMeshFile("cube.msh");
	sphere = Mesh::LoadFromMeshFile("sphere.msh");
	heightmap = new HeightMap(TEXTUREDIR"noise1.png", TEXTUREDIR"noise2.png", TEXTUREDIR"noise3.png", TEXTUREDIR"squareGradient.png");

	//rotation[0] = Matrix4::Rotation(0, Vector3(0, 1, 0));
	//rotation[1] = Matrix4::Rotation(90, Vector3(0, 1, 0));
	//rotation[2] = Matrix4::Rotation(180, Vector3(0, 1, 0));
	//rotation[3] = Matrix4::Rotation(270, Vector3(0, 1, 0));
	//rotation[4] = Matrix4::Rotation(90, Vector3(1, 0, 0));
	//rotation[5] = Matrix4::Rotation(-90, Vector3(1, 0, 0));

#pragma region Init Textures
	waterTexture = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	waterBump = SOIL_load_OGL_texture(TEXTUREDIR"waterbump.PNG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTexture = SOIL_load_OGL_texture(TEXTUREDIR"grass2.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"grass2bump.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sandTexture = SOIL_load_OGL_texture(TEXTUREDIR"sand.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sandBump = SOIL_load_OGL_texture(TEXTUREDIR"sandBump.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	rockTexture = SOIL_load_OGL_texture(TEXTUREDIR"rock.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	rockBump = SOIL_load_OGL_texture(TEXTUREDIR"rockBump.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	heightmapTex = SOIL_load_OGL_texture(TEXTUREDIR"noise1.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"Daylight Box_Right.bmp", TEXTUREDIR"Daylight Box_Left.bmp",
		TEXTUREDIR"Daylight Box_Top.bmp", TEXTUREDIR"Daylight Box_Bottom.bmp",
		TEXTUREDIR"Daylight Box_Front.bmp", TEXTUREDIR"Daylight Box_Back.bmp",
		
		
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!waterTexture	|| !waterBump		|| 
		!earthTexture	|| !earthBump		|| 
		!cubeMap		|| !heightmapTex	|| 
		!sandTexture	|| !sandBump		|| 
		!rockTexture	|| !rockBump		)
		return;

	SetTextureRepeating(waterTexture, true);
	SetTextureRepeating(waterBump, true);
	SetTextureRepeating(earthTexture, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(sandTexture, true);
	SetTextureRepeating(sandBump, true);
	SetTextureRepeating(rockTexture, true);
	SetTextureRepeating(rockBump, true);
	SetTextureRepeating(heightmapTex, true);

#pragma endregion

#pragma region Init Shaders
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("LightingVertex.glsl", "LightingFragment.glsl");
	defaultShader = new Shader("DefaultVertex.glsl", "DefaultFragment.glsl");
	shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");
	processShader = new Shader("ProcessVertex.glsl", "NoProcessFrag.glsl");
	mapProcessShader = new Shader("ProcessVertex.glsl", "NoProcessFrag.glsl");
	if (!lightShader->LoadSuccess()		|| !defaultShader->LoadSuccess()	||
		!reflectShader->LoadSuccess()	|| !skyboxShader->LoadSuccess()		||
		!shadowShader->LoadSuccess()	|| !processShader->LoadSuccess()	||
		!mapProcessShader->LoadSuccess())
		return;
#pragma endregion

	Vector3 heightmapSize = heightmap->GetHeightMapSize();

	root = new SceneNode();

#pragma region HeightMap Node
	SceneNode* heightMapNode = new SceneNode(heightmap, Vector4(1,1,1,1));
	BoundingBox* b = new BoundingBox(heightmapSize * Vector3(0.5, 0.5, 0.5), heightMapNode->GetWorldTransform());
	b->SetOffset(heightmapSize * 0.5);

	heightMapNode->SetBoundingVolume(b);
	heightMapNode->SetTexture(&earthTexture);
	heightMapNode->SetBumpMapTexture(&earthBump);
	heightMapNode->SetSecondaryTexture(&sandTexture, &sandBump);
	heightMapNode->SetThirdTexture(&rockTexture, &rockBump);
	heightMapNode->SetShadowTex(&shadowTex);
	heightMapNode->SetShader(lightShader);
	root->AddChild(heightMapNode);
#pragma endregion

#pragma region Water Node
	WaterNode* water = new WaterNode(heightmap->GetHeightMapSize(), quad);
	water->SetTransform(
		Matrix4::Translation(heightmapSize * 0.5f) *
		Matrix4::Scale(heightmapSize * 0.5f) *
		Matrix4::Rotation(90, Vector3(1, 0, 0))
	);
	BoundingBox* waterBound = new BoundingBox(heightmapSize, water->GetWorldTransform());
	water->SetBoundingVolume(waterBound);

	heightMapNode->AddChild(water);

	water->SetTexture(&waterTexture);
	water->SetBumpMapTexture(&waterBump);
	water->SetCubeMapTexture(&cubeMap);
	water->SetHeightMapTexture(&heightmapTex);

	water->SetShader(reflectShader);
#pragma endregion

#pragma region Create Shadow Lights / Meshes
	lights.emplace_back(new Light(Vector3(10.0f, 7.5f, 0.0f), Vector4(1, 1, 1, 1), DIRECTIONAL_LIGHT));

	Vector3 lightLocation = Vector3(heightmapSize.x * 0.5 + 50.0f, 50.0f, heightmapSize.z * 0.5 + 50.0f);
	lightLocation = Vector3(lightLocation.x, heightmap->GetHeightAtLocation(lightLocation) + 100.0f, lightLocation.z);

	Vector3 lightLocation2 = Vector3(heightmapSize.x * 0.7 - 50.0f, 50.0f, heightmapSize.z * 0.4 - 50.0f);
	lightLocation2 = Vector3(lightLocation2.x, heightmap->GetHeightAtLocation(lightLocation2) + 100.0f, lightLocation2.z);
	//pointLight = ;
	//pointLight2 = ;
	//lights.emplace_back(new Light(lightLocation2, Vector4(0, 0, 1, 1), POINT_LIGHT, 100000.0f, true));
	//lights.emplace_back(new Light(lightLocation, Vector4(1, 0, 0, 1), POINT_LIGHT, 100000.0f, true));

	SceneNode* testShadow = new SceneNode(sphere);
	testShadow->SetTransform(Matrix4::Translation(lightLocation) * Matrix4::Translation(Vector3(-70, -25, -70)) * Matrix4::Scale(Vector3(10, 10, 10)));
	testShadow->SetShader(lightShader);
	BoundingBox* shadowBound = new BoundingBox(Vector3(50, 50, 50), testShadow->GetWorldTransform());
	testShadow->SetBoundingVolume(shadowBound);
	heightMapNode->AddChild(testShadow);

	SceneNode* testShadow2 = new SceneNode(debugCube);
	testShadow2->SetTransform(Matrix4::Translation(lightLocation2) * Matrix4::Translation(Vector3(-70, -25, -70)) * Matrix4::Scale(Vector3(15, 15, 15)));
	testShadow2->SetShader(lightShader);
	BoundingBox* shadowBound2 = new BoundingBox(Vector3(50, 50, 50), testShadow2->GetWorldTransform());
	testShadow2->SetBoundingVolume(shadowBound2);
	heightMapNode->AddChild(testShadow2);
#pragma endregion

#pragma region Create Lights

	for (int i = 0; i < 20; i++) {
		Vector4 col = Vector4((rand() % 10) / 10.0f, (rand() % 10) / 10.0f, (rand() % 10) / 10.0f, 1);
		SceneNode* lightNode = new SceneNode(sphere, col);
		Vector3 location;
		while (true) {
			location = Vector3(rand() % (int)heightmapSize.x,
				350.0f,
				rand() % (int)heightmapSize.z);

			float height = heightmap->GetHeightAtLocation(location);

			if (height > 150.0f) {
				location.y = height + 30.0f;
				break;
			}
		}
		lightNode->SetTransform(
			Matrix4::Translation(location) *
			Matrix4::Scale(Vector3(10, 10, 10))
		);

		BoundingBox* b = new BoundingBox(Vector3(50, 50, 50), lightNode->GetWorldTransform());
		lightNode->SetBoundingVolume(b);
		lightNode->SetShader(defaultShader);
		heightMapNode->AddChild(lightNode);
		Light* l = new Light(lightNode->GetTransform().GetPositionVector(), col, POINT_LIGHT);
		lightNode->SetLight(l);
		lightNode->SetShadowTex(&shadowTex);
		lights.emplace_back(l);
	}
#pragma endregion

#pragma region Create Shadow FBO
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ------------------------------------

	//glGenTextures(1, &shadowCubeTex);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubeTex);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//for (int i = 0; i < 6; i++) {
	//	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//}

	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	//glGenFramebuffers(1, &shadowCubeFBO);
	//glBindFramebuffer(GL_FRAMEBUFFER, shadowCubeFBO);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowCubeTex, 0);
	//glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

#pragma endregion

#pragma region Create FBOs
	// Create Scene Depth Texture
	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	// Create colour texture
	for (int i = 0; i < 2; i++) {
		glGenTextures(1, &bufferColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	// Create Scene Depth Texture
	glGenTextures(1, &mapDepthTex);
	glBindTexture(GL_TEXTURE_2D, mapDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	// Create colour texture
	for (int i = 0; i < 2; i++) {
		glGenTextures(1, &mapColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, mapColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &mapFBO);
	glGenFramebuffers(1, &processFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);

	glBindFramebuffer(GL_FRAMEBUFFER, mapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mapDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mapDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mapColourTex[0], 0);

	// Check success
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColourTex[0] || !mapColourTex[0]) {
		return;
	}
#pragma endregion

	camera = new Camera(-45.0f, 0.0f, 0.0f, heightmapSize * Vector3(0.5f, 2.0f, 0.5f));
	mapView = new Camera(-90, 0.0f, 0, heightmapSize * Vector3(0.5f, 5.0f, 0.5f), true);

	//projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

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

	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
}

// TODO: Cubemap lighting

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	mapView->UpdateCamera(dt);
	//viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	time += dt;
	
	//Vector3 newPos = Vector3(pointLight->GetPosition().x, pointLight->GetPosition().y + (sin(time) / 10.0f), pointLight->GetPosition().z);
	//pointLight->SetPosition(newPos);

	//for (auto light : lights) {
	//	if (light->GetType() == POINT_LIGHT) {
	//		light->SetPosition(light->GetPosition() + Vector3(0, sin(time) / 2.0f, 0));
	//	}
	//	//transform = transform * Matrix4::Translation(Vector3(0, , 0));
	//}

	mapViewMatrix = mapView->BuildViewMatrix();
	sceneViewMatrix = camera->BuildViewMatrix();

	root->Update(dt);
}
// TODO reflect shader
void Renderer::BuildNodeLists(SceneNode* from) {
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->IsLight()) {
			lightNodeList.push_back(from);
		}
		else {
			if (from->GetColour().w < 1.0f) {
				transparentNodeList.push_back(from);
			}
			else {
				nodeList.push_back(from);
			}
		}
	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists(*i);
	}
}

void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.rbegin(), transparentNodeList.rend(), SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
	std::sort(lightNodeList.begin(), lightNodeList.end(), SceneNode::CompareByCameraDistance);

	//std::cout << "No. Lights: " << lights.size() << std::endl;
}

void Renderer::DrawNodes(bool shadowPass) {

	for (const auto& i : nodeList) {
		DrawNode(i, shadowPass);
	}

	for (const auto& i : lightNodeList) {
		DrawNode(i, shadowPass);
	}

	for (const auto& i : transparentNodeList) {
		DrawNode(i, shadowPass);
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

void Renderer::DrawNode(SceneNode* n, bool shadowPass) {
	if (n->GetMesh()) {
		if (!shadowPass) {
			Shader* currentShader = n->GetShader();
			BindShader(currentShader);
			SetShaderLights(lights);
			glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
			glUniform4fv(glGetUniformLocation(currentShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
			glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "shadowTex"), 6);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, shadowTex);
			n->SetShaderVariables();
			textureMatrix = n->GetTextureMat();
		}
		modelMatrix = n->GetModelMat();
		UpdateShaderMatrices();

		if (!shadowPass) {
			n->Draw(*this);
		}
		else {
			if (!n->CheckShadowExempt()) {
				n->Draw(*this);
			}
		}
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
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glEnable(GL_DEPTH_TEST);

	//DrawShadowScene();

	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	glClearColor(0, 0, 0, 1.0f);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	viewMatrix = sceneViewMatrix;

	DrawMainScene();

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);

	DrawSkyBox();
	
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	DrawPostProcess(bufferColourTex, processShader);

	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, mapFBO);
	glViewport(0, 0, width, height);
	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	projMatrix = Matrix4::Orthographic(1.0f, 10000.0f, height / 2.0f, -height / 2.0f, height / 2.0f, -height / 2.0f);
	viewMatrix = mapViewMatrix;

	DrawMainScene();
	DrawPostProcess(mapColourTex, mapProcessShader, 1);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	viewMatrix = sceneViewMatrix;

	PresentScene();
	
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
	lightNodeList.clear();
}

void Renderer::DrawShadowScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	int counter = 0;
	int xOffset = 0; int yOffset = 0;
	int size = 2048;
	for (auto light : lights) {
		if (light->CheckCastShadows()) {
			glViewport(0 + xOffset, 0 + yOffset, size, size);

			xOffset += size;
			if (xOffset == SHADOWSIZE) {
				xOffset = 0;
				yOffset += size;
			}

			BindShader(shadowShader);
			viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(0, 0, 0));
			projMatrix = Matrix4::Perspective(1, 1000, 1, 90);
			light->SetShadowMatrix(projMatrix * viewMatrix);

			DrawNodes(true);
		}
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawMainScene() {
	DrawNodes();
}

void Renderer::DrawPostProcess(GLuint* textureArray, Shader* processShader, int numberPasses) {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureArray[1], 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(processShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(processShader->GetProgram(), "sceneTex"), 0);
	for (int i = 0; i < numberPasses; i++) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureArray[1], 0);
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 0);

		glBindTexture(GL_TEXTURE_2D, textureArray[0]);
		quad->Draw();

		//Swap
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureArray[0], 0);

		glBindTexture(GL_TEXTURE_2D, textureArray[1]);
		quad->Draw();
	}
}

void Renderer::PresentScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(defaultShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	// Render full screen scene
	glViewport(0, 0, width, height);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
	glUniform1i(glGetUniformLocation(defaultShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(defaultShader->GetProgram(), "useTexture"), 1);

	quad->Draw();

	// Render map in corner
	glViewport(0, height - 200, 200, 200);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mapColourTex[0]);
	glUniform1i(glGetUniformLocation(defaultShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(defaultShader->GetProgram(), "useTexture"), 1);
	
	circle->Draw();

}