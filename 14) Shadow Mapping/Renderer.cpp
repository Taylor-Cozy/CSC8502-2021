#include "Renderer.h"

#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	camera = new Camera(-30.0f, 315.0f, 0.0f, Vector3(-8.0f, 5.0f, 8.0f));

	lights.emplace_back(new Light(Vector3(-20.0f, 10.0f, -20.0f), Vector4(1, 0, 0, 1), POINT_LIGHT, true));
	lights.emplace_back(new Light(Vector3(20.0f, 10.0f, -20.0f), Vector4(0, 0, 1, 1), POINT_LIGHT, true));
	lights.emplace_back(new Light(Vector3(0.0f, 10.0f, 20.0f), Vector4(0, 1, 0, 1), POINT_LIGHT, true));

	sceneShader = new Shader("shadowSceneVert.glsl", "shadowSceneFrag.glsl");
	shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");

	if (!sceneShader->LoadSuccess() || !shadowShader->LoadSuccess())
		return;

	sceneMeshes.emplace_back(Mesh::GenerateQuad());
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Sphere.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Cylinder.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Cone.msh"));

	sceneDiffuse = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sceneBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	SetTextureRepeating(sceneDiffuse, true);
	SetTextureRepeating(sceneBump, true);

	glEnable(GL_DEPTH_TEST);

	sceneTransforms.resize(4);
	sceneTransforms[0] = Matrix4::Rotation(90, Vector3(1, 0, 0)) * Matrix4::Scale(Vector3(10, 10, 1));

	sceneTime = 0.0f;
	init = true;
}

Renderer::~Renderer(void) {
	for (auto i : lights) {
		delete i;
	}

	for (auto& i : sceneMeshes) {
		delete i;
	}

	delete camera;
	delete sceneShader;
	delete shadowShader;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	sceneTime += dt;

	for (int i = 1; i < 4; ++i) {
		Vector3 t = Vector3(-10 + (5 * i), 2.0f + sin(sceneTime * i), 0);
		sceneTransforms[i] = Matrix4::Translation(t) * Matrix4::Rotation(sceneTime * 10 * i, Vector3(1, 0, 0));
	}
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawShadowScene();
	DrawMainScene();
}

void Renderer::DrawShadowScene() {
	int counter = 0;
	for (auto light : lights) {
		glBindFramebuffer(GL_FRAMEBUFFER, light->getShadowFBO());

		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		BindShader(shadowShader);
		viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(0, 0, 0));
		projMatrix = Matrix4::Perspective(1, 100, 1, 45);
		light->SetShadowMatrix(projMatrix * viewMatrix);

		for (int i = 0; i < 4; i++) {
			modelMatrix = sceneTransforms[i];
			UpdateShaderMatrices();
			sceneMeshes[i]->Draw();
		}

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glViewport(0, 0, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void Renderer::DrawMainScene() {
	BindShader(sceneShader);
	SetShaderLights(lights);

	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width/(float)height, 45.0f);

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex"), 2);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex2"), 3);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex3"), 4);
	glUniform3fv(glGetUniformLocation(sceneShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneDiffuse);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, sceneBump);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, lights.at(0)->getShadowTex());

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, lights.at(1)->getShadowTex());

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, lights.at(2)->getShadowTex());



	GLuint shadowTextures[3];
	for (int i = 0; i < 3; i++) {
		shadowTextures[0] = lights[0]->getShadowTex();
	}
	glBindTextures(GL_TEXTURE_2D_ARRAY, 3, shadowTextures);

	for (int i = 0; i < 4; i++) {
		modelMatrix = sceneTransforms[i];
		UpdateShaderMatrices();
		sceneMeshes[i]->Draw();
	}
}