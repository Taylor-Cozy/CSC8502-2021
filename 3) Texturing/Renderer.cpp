#include "Renderer.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	triangle = Mesh::GenerateTriangle();
	cam = new Camera();

	texture = SOIL_load_OGL_texture(TEXTUREDIR"brick.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	secondaryTexture = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	if (!texture || !secondaryTexture) {
		return;
	}

	shader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");

	if (!shader->LoadSuccess())
		return;

	filtering = true;
	repeating = true;
	init = true;

	SwitchToPerspective();
}

Renderer::~Renderer(void) {
	delete triangle;
	delete shader;
	delete cam;
	glDeleteTextures(1, &texture);
	glDeleteTextures(1, &secondaryTexture);
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	BindShader(shader);
	UpdateShaderMatrices();

	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "projMatrix"), 1, false, projMatrix.values);
	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "viewMatrix"), 1, false, viewMatrix.values);

	modelMatrix = Matrix4::Translation(Vector3(0, 0, 0)) *
		Matrix4::Rotation(rotation, Vector3(0, 1, 0)) *
		Matrix4::Scale(Vector3(scale, scale, scale));

	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, modelMatrix.values);

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "secondTex"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, secondaryTexture);

	triangle->Draw();
}

void Renderer::UpdateTextureMatrix(float value) {
	Matrix4 push = Matrix4::Translation(Vector3(-0.5f, -0.5f, 0));
	Matrix4 pop = Matrix4::Translation(Vector3(0.5f, 0.5f, 0));
	Matrix4 rotation = Matrix4::Rotation(value, Vector3(0, 0, 1));
	textureMatrix = pop * rotation * push;
}

void Renderer::ToggleRepeating() {
	repeating = !repeating;
	SetTextureRepeating(texture, repeating);
	SetTextureRepeating(secondaryTexture, repeating);
}

void Renderer::ToggleFiltering() {
	filtering = !filtering;
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering ? GL_LINEAR : GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, secondaryTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering ? GL_LINEAR : GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 1);
}

void Renderer::SwitchToPerspective()
{
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
}

void Renderer::UpdateScene(float dt) {
	cam->UpdateCamera(dt);
	viewMatrix = cam->BuildViewMatrix();
}