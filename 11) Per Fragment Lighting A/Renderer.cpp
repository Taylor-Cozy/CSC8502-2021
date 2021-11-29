#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"

const int MAX_LIGHTS = 64;

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	heightMap = new HeightMap(TEXTUREDIR"noise1.png", TEXTUREDIR"noise2.png", TEXTUREDIR"noise3.png", TEXTUREDIR"squareGradient.png");
	texture = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	shader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
	sphere = Mesh::LoadFromMeshFile("cube.msh");

	if (!shader->LoadSuccess() || !texture || !sphere) {
		return;
	}

	SetTextureRepeating(texture, true);

	Vector3 heightmapSize = heightMap->GetHeightMapSize();
	camera = new Camera(-45.0f, 0.0f, 0.0f, heightmapSize * Vector3(0.5f, 5.0f, 0.5f));

	lights.push_back(new Light(Vector3(10, 10, 0), Vector4(1, 1, 0.7, 1), Vector4(1, 1, 1, 1), DIRECTIONAL_LIGHT));
	for (int i = 0; i < 4; i++) {
		lights.push_back(new Light(heightmapSize * Vector3(0.0f + (0.2f * (i+ 1)), 1.5f, 0.5f), Vector4(1, 1, 1, 1), Vector4(1, 0, 0, 1), POINT_LIGHT));
	}

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	init = true;
}

Renderer::~Renderer(void) {
	delete camera;
	delete heightMap;
	delete shader;
	for (auto x : lights) {
		delete x;
	}
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	Vector3 currentSunPos = lights.at(0)->GetPosition();
	//currentSunPos.x -=  dt;
	lights.at(0)->SetPosition(currentSunPos);
}

void Renderer::RenderScene() {
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(shader);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glUniform3fv(glGetUniformLocation(shader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	
	UpdateShaderMatrices();
	SetShaderLights(lights);
	heightMap->Draw();
}