#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Heightmap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/Camera.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	quad = Mesh::GenerateQuad();
	circle = Mesh::GenerateCircle();

	heightMap = new HeightMap(TEXTUREDIR"noise1.png", TEXTUREDIR"noise2.png", TEXTUREDIR"noise3.png", TEXTUREDIR"squareGradient.png");

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	waterBump = SOIL_load_OGL_texture(TEXTUREDIR"waterbump.PNG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	heightmapTex = SOIL_load_OGL_texture(TEXTUREDIR"noise.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg",
		TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg",
		TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!earthTex || !earthBump || !cubeMap || !waterTex || !waterBump)
		return;

	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(waterBump, true);
	SetTextureRepeating(heightmapTex, true);

	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
	mirrorShader = new Shader("MirrorVertex.glsl", "MirrorFragment.glsl");
	

	if (!reflectShader->LoadSuccess())
		return;
	if (!skyboxShader->LoadSuccess())
		return;
	if (!lightShader->LoadSuccess())
		return;
	if (!mirrorShader->LoadSuccess())
		return;

	Vector3 heightmapSize = heightMap->GetHeightMapSize();

	camera = new Camera(-45.0f, 0.0f, 0.0f, heightmapSize * Vector3(0.5f, 5.0f, 0.5f));
	lights.emplace_back(new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1, 1, 1, 1), Vector4(1, 1, 1, 1), DIRECTIONAL_LIGHT));

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	waterRotate = 0.0f;
	waterCycle = 0.0f;
	time = 0;
	init = true;
}

Renderer::~Renderer(void) {
	delete camera;
	delete heightMap;
	delete quad;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	for (auto x : lights) {
		delete x;
	}
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	waterRotate += dt * .20f;
	waterCycle += dt * 0.025f;
	time += dt;
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	DrawHeightMap();
	DrawWater();
	DrawMirror();
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	DrawSkyBox();
	// Not sure why this needs to be here but without it, the stencil buffer does not reset between frames
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
}

void Renderer::DrawSkyBox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightMap() {
	BindShader(lightShader);
	SetShaderLights(lights);

	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	heightMap->Draw();
}

void Renderer::DrawWater() {
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "heightmapTex"), 3);

	glUniform1f(glGetUniformLocation(reflectShader->GetProgram(), "time"), time);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, waterBump);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, heightmapTex);

	Vector3 hSize = heightMap->GetHeightMapSize();

	modelMatrix =
		Matrix4::Translation(hSize * 0.5f) * Matrix4::Translation(Vector3(0,sin(time),0)) *
		Matrix4::Scale(hSize * 0.5f) *
		Matrix4::Rotation(90, Vector3(1, 0, 0));

	textureMatrix =
		Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
		Matrix4::Scale(Vector3(10, 10, 10)) *
		Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	quad->Draw();
}

void Renderer::DrawMirror() {
	BindShader(mirrorShader);

	glUniform3fv(glGetUniformLocation(mirrorShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(mirrorShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(mirrorShader->GetProgram(), "cubeTex"), 2);
	glUniform1i(glGetUniformLocation(mirrorShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(mirrorShader->GetProgram(), "heightmapTex"), 3);

	glUniform1f(glGetUniformLocation(mirrorShader->GetProgram(), "time"), time);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, waterBump);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, heightmapTex);

	Vector3 hSize = heightMap->GetHeightMapSize();

	Vector3 heightMapSize = heightMap->GetHeightMapSize();

	modelMatrix = 
		Matrix4::Translation(heightMapSize * 0.5f * Vector3(1,3,1)) * 
		Matrix4::Scale(Vector3(75, 150, 1));

	UpdateShaderMatrices();
	circle->Draw();
}