#include "Renderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/BoundingBox.h"
#include "../nclgl/WaterNode.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Light.h"
#include "..//nclgl/MeshAnimation.h"
#include "..//nclgl/MeshMaterial.h"

#define SHADOWSIZE 4096

Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{
	
	quad = Mesh::GenerateQuad();
	circle = Mesh::GenerateCircle();
	debugCube = Mesh::LoadFromMeshFile("cube.msh");
	sphere = Mesh::LoadFromMeshFile("sphere.msh");
	heightmap = new HeightMap(TEXTUREDIR"noise1.png", TEXTUREDIR"noise2.png", TEXTUREDIR"noise3.png", TEXTUREDIR"squareGradient.png");
	soldier = Mesh::LoadFromMeshFile("Role_T.msh");
	soldierAnim = new MeshAnimation("Role_T.anm");
	soldierMaterial = new MeshMaterial("Role_T.mat");
	tree = Mesh::LoadFromMeshFile("palmtree.msh");

	for (int i = 0; i < soldier->GetSubMeshCount(); i++) {
		const MeshMaterialEntry* matEntry = soldierMaterial->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		soldierTextures.emplace_back(texID);
	}

	currentFrame = 0;
	frameTime = 0.0f;

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

	cubeMapMountains = SOIL_load_OGL_cubemap(
		TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg",
		TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg",
		TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	currentCubeMap = &cubeMap;

	if (!waterTexture	|| !waterBump		|| 
		!earthTexture	|| !earthBump		|| 
		!cubeMap		|| !heightmapTex	|| 
		!sandTexture	|| !sandBump		|| 
		!rockTexture	|| !rockBump		||
		!cubeMapMountains)
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
	processShader = new Shader("ProcessVertex.glsl", "BlurFrag.glsl");
	mapProcessShader = new Shader("ProcessVertex.glsl", "NoProcessFrag.glsl");
	bloomShader = new Shader("ProcessVertex.glsl", "BloomFrag.glsl");
	combineBloomShader = new Shader("ProcessVertex.glsl", "CombineBloomFrag.glsl");
	skinningShader = new Shader("SkinningVertex.glsl", "SkinningFragment.glsl");
	mirrorShader = new Shader("MirrorVertex.glsl", "MirrorFragment.glsl");
	if (!lightShader->LoadSuccess()			|| !defaultShader->LoadSuccess()	||
		!reflectShader->LoadSuccess()		|| !skyboxShader->LoadSuccess()		||
		!shadowShader->LoadSuccess()		|| !processShader->LoadSuccess()	||
		!mapProcessShader->LoadSuccess()	|| !bloomShader->LoadSuccess()		||
		!combineBloomShader->LoadSuccess()	|| !skinningShader->LoadSuccess()	||
		!mirrorShader->LoadSuccess())
		return;
#pragma endregion

	Vector3 heightmapSize = heightmap->GetHeightMapSize();

	root = new SceneNode();

#pragma region HeightMap Node
	SceneNode* heightMapNode = new SceneNode(heightmap, Vector4(1,1,1,1));
	BoundingBox* b = new BoundingBox(heightmapSize * 0.5f, heightMapNode->GetWorldTransform());
	b->SetOffset(heightmapSize * 0.5f);

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
	WaterNode* water = new WaterNode(heightmap->GetHeightMapSize(), &sunTime, quad);
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
	water->SetMtnCubeMapTexture(&cubeMapMountains);
	water->SetHeightMapTexture(&heightmapTex);

	water->SetShader(reflectShader);
#pragma endregion

#pragma region Create Shadow Lights / Meshes
	lights.emplace_back(new Light(dirLight, Vector4(1, 1, 1, 1), DIRECTIONAL_LIGHT));

	Vector3 lightLocation = Vector3(heightmapSize.x * 0.5 + 50.0f, 50.0f, heightmapSize.z * 0.5 + 50.0f);
	lightLocation = Vector3(lightLocation.x, heightmap->GetHeightAtLocation(lightLocation) + 100.0f, lightLocation.z);

	Vector3 lightLocation2 = Vector3(heightmapSize.x * 0.7 - 50.0f, 50.0f, heightmapSize.z * 0.4 - 50.0f);
	lightLocation2 = Vector3(lightLocation2.x, heightmap->GetHeightAtLocation(lightLocation2) + 100.0f, lightLocation2.z);
	pointLight = new Light(lightLocation2, Vector4(0, 0, 1, 1), POINT_LIGHT, 100000.0f, true);
	pointLight2 = new Light(lightLocation, Vector4(1, 0, 0, 1), POINT_LIGHT, 100000.0f, true);
	lights.emplace_back(pointLight);
	lights.emplace_back(pointLight2);

	SceneNode* testShadow = new SceneNode(sphere);
	testShadow->SetTransform(Matrix4::Translation(lightLocation) * Matrix4::Translation(Vector3(-70, -25, -70)) * Matrix4::Scale(Vector3(10, 10, 10)));
	testShadow->SetShader(lightShader);
	BoundingBox* shadowBound = new BoundingBox(Vector3(10, 10, 10), testShadow->GetWorldTransform());
	testShadow->SetBoundingVolume(shadowBound);
	heightMapNode->AddChild(testShadow);

	SceneNode* testShadow2 = new SceneNode(debugCube);
	testShadow2->SetTransform(Matrix4::Translation(lightLocation2) * Matrix4::Translation(Vector3(-70, -25, -70)) * Matrix4::Scale(Vector3(15, 15, 15)));
	testShadow2->SetShader(lightShader);
	BoundingBox* shadowBound2 = new BoundingBox(Vector3(15, 15, 15), testShadow2->GetWorldTransform());
	testShadow2->SetBoundingVolume(shadowBound2);
	heightMapNode->AddChild(testShadow2);

#pragma endregion

#pragma region Create Lights

	for (int i = 0; i < 25; i++) {
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

		BoundingSphere* b = new BoundingSphere(10.0f, lightNode->GetWorldTransform());
		lightNode->SetBoundingVolume(b);
		lightNode->SetShader(defaultShader);
		heightMapNode->AddChild(lightNode);
		Light* l = new Light(lightNode->GetTransform().GetPositionVector(), col, POINT_LIGHT);
		lightNode->SetLight(l);
		lightNode->SetShadowTex(&shadowTex);
		lights.emplace_back(l);
	}
#pragma endregion

	SceneNode* treeNode = new SceneNode(tree);
	treeNode->SetTransform(Matrix4::Translation(Vector3(0, 0, 0)) * Matrix4::Scale(Vector3(2.0, 2.0, 2.0)));
	treeNode->SetShader(defaultShader);
	heightMapNode->AddChild(treeNode);

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

	// Create colour texture
	for (int i = 0; i < 3; i++) {
		glGenTextures(1, &bloomColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, bloomColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &mapFBO);
	glGenFramebuffers(1, &processFBO);
	glGenFramebuffers(1, &bloomFBO);

	GLenum buffers[3] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2
	};

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);

	glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomColourTex[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bloomColourTex[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, bloomColourTex[2], 0);
	glDrawBuffers(3, buffers);

	glBindFramebuffer(GL_FRAMEBUFFER, mapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mapDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mapDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mapColourTex[0], 0);

	// Check success
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColourTex[0] || !mapColourTex[0]) {
		return;
	}
#pragma endregion

	//SceneNode* mirror = new SceneNode(circle);
	//mirror->SetShader(mirrorShader);
	//mirror->SetTexture(&cubeMapMountains);
	//mirror->SetTransform(heightMapNode->GetTransform() * Matrix4::Translation(Vector3(0, 300, 0)));
	////mirror->Set

	camera = new Camera(-45.0f, 0.0f, 0.0f, heightmapSize * Vector3(0.05f, 3.0f, 0.05f));
	mapView = new Camera(-90, 0.0f, 0, heightmapSize * Vector3(0.05f, 5.0f, 0.05f), true);

	//projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	waypoints.emplace_back(heightmapSize * Vector3(0.05f, 3.0f, 0.05f));	// Entire Island
	waypoints.emplace_back(heightmapSize * Vector3(0.05f, 0.1f, 0.2f));		// Ocean
	waypoints.emplace_back(heightmapSize * Vector3(0.2f, 0.1f, 0.9f));		// Ocean
	waypoints.emplace_back(heightmapSize* Vector3(0.5f, 0.7f, 0.55f));		// Beach Soldier + Real Time Shadow
	waypoints.emplace_back(heightmapSize* Vector3(0.58f, 0.9f, 0.4f));		// Other real time shadow
	waypoints.emplace_back(heightmapSize* Vector3(0.61f, 0.5f, 0.42f));		// Bloom
	waypoints.emplace_back(heightmapSize* Vector3(0.35f, 0.8f, 0.27f));		// Portal
	waypoints.emplace_back(heightmapSize* Vector3(0.25f, 0.8f, 0.22f));		// Portal
	waypointRot.emplace_back(Vector2(-30.0f, -130));
	waypointRot.emplace_back(Vector2(-0.0f, -170));
	waypointRot.emplace_back(Vector2(-0.0f, -50));
	waypointRot.emplace_back(Vector2(-20.0f, 10));
	waypointRot.emplace_back(Vector2(-20.0f, -80));
	waypointRot.emplace_back(Vector2(-20.0f, -180));
	waypointRot.emplace_back(Vector2(-5.0f, -220));
	waypointRot.emplace_back(Vector2(-5.0f, -150));
	waypointTimes.emplace_back(5.0f);
	waypointTimes.emplace_back(0.0f);
	waypointTimes.emplace_back(0.0f);
	waypointTimes.emplace_back(5.0f);
	waypointTimes.emplace_back(3.0f);
	waypointTimes.emplace_back(3.0f);
	waypointTimes.emplace_back(0.1f);
	waypointTimes.emplace_back(3.0f);

	oldPos = camera->GetPosition();
	oldRot = Vector2(-45.0f, 0);

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

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	mapView->UpdateCamera(dt);
	time += dt;

	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % soldierAnim->GetFrameCount();
		frameTime += 1.0f / soldierAnim->GetFrameRate();
	}

	if (followTrack) {
		if (curWaitTime <= 0.0f) {
			curTime += dt;
			float t = curTime / waypointTime;

			Vector3 newcamPos = lerp(waypoints[currentWaypoint], oldPos, t);
			float newYaw = lerp(waypointRot[currentWaypoint].y, oldRot.y, t);
			float newPitch = lerp(waypointRot[currentWaypoint].x, oldRot.x, t);
			camera->SetPosition(newcamPos);
			camera->SetYaw(newYaw);
			camera->SetPitch(newPitch);
			mapView->SetPosition(Vector3(newcamPos.x, mapView->GetPosition().y, newcamPos.z));
			mapView->SetYaw(newYaw);

			if (distance(camera->GetPosition(), waypoints[currentWaypoint]) < 10.0f) {
				curWaitTime = waypointTimes[currentWaypoint];
				oldPos = waypoints[currentWaypoint];
				oldRot = waypointRot[currentWaypoint];
				currentWaypoint++;
				if (currentWaypoint == waypoints.size())
					currentWaypoint = 0;
				std::cout << "Waypoint Met." << std::endl;
				curTime = 0.0f;
			}
		}
		else {
			curWaitTime -= dt;
			std::cout << curWaitTime << std::endl;
		}

	}

	if (changeSkybox) {
		if (sunTime <= 0.0f) {
			sunTime = 0.0f;
		}
		else {
			sunTime -= dt / 3.0f;
		}
	}
	else {
		if (sunTime >= 1.0f) {
			sunTime = 1.0f;
		}
		else {
			sunTime += dt / 3.0f;
		}
	}

	Vector3 newPos = Vector3(pointLight->GetPosition().x, pointLight->GetPosition().y + (sin(time) / 10.0f), pointLight->GetPosition().z);
	pointLight->SetPosition(newPos);

	for (auto light : lights) {
		if (light->GetType() == POINT_LIGHT) {
			light->SetPosition(light->GetPosition() + Vector3(0, sin(time) / 2.0f, 0));
		}
		//transform = transform * Matrix4::Translation(Vector3(0, , 0));
	}

	mapViewMatrix = mapView->BuildViewMatrix();
	sceneViewMatrix = camera->BuildViewMatrix();

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	frameFrustum.FromMatrix(projMatrix * sceneViewMatrix);

	root->Update(dt);
}


// TODO reflect shader
void Renderer::BuildNodeLists(SceneNode* from) {
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		//if (from->IsLight()) {
		//	lightNodeList.push_back(from);
		//}
		//else {
			if (from->GetColour().w < 1.0f) {
				transparentNodeList.push_back(from);
			}
			else {
				nodeList.push_back(from);
			}
		//}
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
	glUniform4fv(glGetUniformLocation(defaultShader->GetProgram(), "nodeColour"), 1, (float*)&Vector4(1, 1, 1, 1));
	glUniform1i(glGetUniformLocation(defaultShader->GetProgram(), "useTexture"), 0);
	UpdateShaderMatrices();

	//glDisable(GL_DEPTH_TEST);

	for (const auto& i : nodeList) {
		DrawDebugNode(i);
	}

	for (const auto& i : lightNodeList) {
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
		glUniform4fv(glGetUniformLocation(defaultShader->GetProgram(), "nodeColour"), 1, (float*)& Vector4(0.98, 0.28, 0.76, 0.5));
		
		if (n->GetBoundingVolume()->GetType() == BOX) {
			//Matrix4 model = n->GetBoundingVolume()->GetWorldPosition() * Matrix4::Scale(Vector3(2, 2, 2)); // Cube is 0.5 units wide therefore scale by 2
			//glUniformMatrix4fv(glGetUniformLocation(defaultShader->GetProgram(), "modelMatrix"), 1, false, (model).values);
			modelMatrix = n->GetBoundingVolume()->GetWorldPosition() * Matrix4::Scale(Vector3(2, 2, 2));
			UpdateShaderMatrices();
			debugCube->Draw();
		}
		else {
			Matrix4 model = n->GetBoundingVolume()->GetWorldPosition(); // Cube is 0.5 units wide therefore scale by 2
			glUniformMatrix4fv(glGetUniformLocation(defaultShader->GetProgram(), "modelMatrix"), 1, false, (model).values);
			sphere->Draw();
		}
		//debugSphere->Draw();
	}
}

void Renderer::DrawMainScene() {
	DrawNodes();
}

void Renderer::RenderScene() {

	BuildNodeLists(root);
	SortNodeLists();

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);

	DrawShadowScene();

	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	glClearColor(0, 0, 0, 0.0f);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	viewMatrix = sceneViewMatrix;

	DrawMainScene();
	DrawSoldier();
	DrawMirror();
	DrawSun();

	//glDisable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//DrawDebugNodes();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glEnable(GL_DEPTH_TEST);

	DrawBloom();

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	viewMatrix = sceneViewMatrix;

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);

	DrawSkyBox();

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	CombineBloom();

	//projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	//viewMatrix = sceneViewMatrix;

	if (camera->GetPosition().y < 140) {
		DrawPostProcess(bufferColourTex, processShader, 10, Vector4(0.98, 0.98, 0.999, 1));
	}
	else {
		DrawPostProcess(bufferColourTex, mapProcessShader);
	}
	
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, mapFBO);
	glViewport(0, 0, width, height);
	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	projMatrix = Matrix4::Orthographic(1.0f, 10000.0f, height / 2.0f, -height / 2.0f, height / 2.0f, -height / 2.0f);
	viewMatrix = mapViewMatrix;

	DrawMainScene();
	DrawPostProcess(mapColourTex, mapProcessShader);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	viewMatrix = sceneViewMatrix;

	PresentScene();

	ClearNodeLists();
}

void Renderer::DrawSkyBox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	glUniform1i(glGetUniformLocation(skyboxShader->GetProgram(), "cubeTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	glUniform1i(glGetUniformLocation(skyboxShader->GetProgram(), "mtnTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapMountains);

	glUniform1f(glGetUniformLocation(skyboxShader->GetProgram(), "sunTime"), sunTime);
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
	glClearColor(0, 0, 0, 0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	int counter = 0;
	int xOffset = 0; int yOffset = 0;
	int size = 1024;
	for (auto light : lights) {
		if (light->CheckCastShadows()) {
			glViewport(0 + xOffset, 0 + yOffset, size, size);

			BindShader(shadowShader);
			viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(0, 0, 0));
			projMatrix = Matrix4::Perspective(1, 1000, 1, 90);
			light->SetShadowMatrix(projMatrix * viewMatrix);
			light->SetShadowOffset(Vector2(xOffset / (1.0f * SHADOWSIZE), yOffset / (1.0f * SHADOWSIZE)));

			xOffset += size;
			if (xOffset == SHADOWSIZE) {
				xOffset = 0;
				yOffset += size;
			}

			DrawNodes(true);
			DrawSoldier();
		}
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawPostProcess(GLuint* textureArray, Shader* processShader, int numberPasses, Vector4 colour) {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureArray[1], 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glClearColor(0, 0, 0, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(processShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(processShader->GetProgram(), "sceneTex"), 0);

	glUniform4fv(glGetUniformLocation(processShader->GetProgram(), "colour"), 1, (float*)&colour);
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

void Renderer::DrawBloom()
{
	glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
	BindShader(bloomShader);
	projMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUniform1i(glGetUniformLocation(bloomShader->GetProgram(), "sceneTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
	quad->Draw();

	DrawPostProcess(bloomColourTex, processShader, 10);
}

void Renderer::CombineBloom()
{
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	BindShader(combineBloomShader);
	projMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();
	glActiveTexture(GL_TEXTURE0);
	bloomColourTex[2] = bufferColourTex[0];
	glBindTexture(GL_TEXTURE_2D, bloomColourTex[2]);
	glUniform1i(glGetUniformLocation(combineBloomShader->GetProgram(), "sceneTex"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bloomColourTex[0]);
	glUniform1i(glGetUniformLocation(combineBloomShader->GetProgram(), "bloomBlur"), 1);
	quad->Draw();
}

void Renderer::DrawSoldier()
{
	BindShader(skinningShader);
	glUniform1i(glGetUniformLocation(skinningShader->GetProgram(), "diffuseTex"), 0);
	Vector3 heightmapSize = heightmap->GetHeightMapSize();
	Vector3 location = (heightmapSize * Vector3(0.5f, 1.0f, 0.5f)) + Vector3(-150, -25, -80);
	location.y = heightmap->GetHeightAtLocation(location);
	modelMatrix = Matrix4::Translation(location) * Matrix4::Scale(Vector3(30, 30, 30));
	UpdateShaderMatrices();

	vector<Matrix4> frameMatrices;

	const Matrix4* invBindPose = soldier->GetInverseBindPose();
	const Matrix4* frameData = soldierAnim->GetJointData(currentFrame);

	for (unsigned int i = 0; i < soldier->GetJointCount(); i++) {
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(skinningShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	for (int i = 0; i < soldier->GetSubMeshCount(); i++) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, soldierTextures[i]);
		soldier->DrawSubMesh(i);
	}
}

void Renderer::DrawMirror() {
	BindShader(mirrorShader);
	glUniform3fv(glGetUniformLocation(mirrorShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(mirrorShader->GetProgram(), "cubeTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapMountains);

	glUniform1i(glGetUniformLocation(mirrorShader->GetProgram(), "skyTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	glUniform1f(glGetUniformLocation(mirrorShader->GetProgram(), "sunTime"), sunTime);

	Vector3 location = heightmap->GetHeightMapSize() * Vector3(0.3f, 2.0f, 0.3f);
	location.y = heightmap->GetHeightAtLocation(location) + 100.0f;

	modelMatrix = Matrix4::Translation(location) * Matrix4::Scale(Vector3(50.0f, 100.0f, 1.0f));

	UpdateShaderMatrices();

	circle->Draw();
}

void Renderer::DrawSun()
{
	BindShader(defaultShader);
	float loc = lerp(2500.0f, -2500.0f, sunTime);
	float rot = lerp(-45, -135, sunTime);
	float dirX = lerp(100.0f, -100.0f, sunTime);
	modelMatrix = Matrix4::Translation(camera->GetPosition() + Vector3(loc, 1875.0f, 0.0f)) * Matrix4::Rotation(90, Vector3(0, 1, 0)) * Matrix4::Rotation(rot, Vector3(1,0,0)) * Matrix4::Scale(Vector3(100, 100, 1));
	dirLight = Vector3(dirX, 75.0f, 0.0f);
	lights[0]->SetPosition(dirLight);
	glUniform4fv(glGetUniformLocation(defaultShader->GetProgram(), "nodeColour"), 1, (float*)&Vector4(1,1,1,1));
	glUniform1i(glGetUniformLocation(defaultShader->GetProgram(), "useTexture"), 0);
	UpdateShaderMatrices();
	circle->Draw();
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