#include "Renderer.h"
#include "../nclgl/CubeRobot.h"
#include "..//nclgl/BoundingVolume.h"
#include "..//nclgl/Camera.h"
#include <algorithm>

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	camera = new Camera(0.0f, 0.0f, 0.0f, (Vector3(0, 100, 750.f)));
	quad = Mesh::GenerateQuad();
	axis = Mesh::DrawAxisLines();
	Mesh* triangle = Mesh::GenerateTriangle();
	cube = Mesh::LoadFromMeshFile("OffsetCubeY.msh");
	debugCube = Mesh::LoadFromMeshFile("cube.msh");

	shader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	debugShader = new Shader("DebugVertex.glsl", "DebugFragment.glsl");

	texture = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);

	if (!shader->LoadSuccess() || !texture)
		return;

	root = new SceneNode();

	for (int i = 0; i < 5; i++) {

		SceneNode* s = new SceneNode();
		s->SetColour(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
		s->SetTransform(Matrix4::Translation(Vector3(0, 100.0f, -300.0f + 100.0f + 100 * i)));
		s->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));
		s->SetBoundingVolume(new BoundingRectangle(100.0f,100.0f,10.0f, s->GetWorldTransform().GetPositionVector()));
		s->SetMesh(quad);
		s->SetTexture(texture);
		root->AddChild(s);
	}
	root->SetMesh(triangle);
	root->SetColour(Vector4(1, 0, 0, 1));
	root->AddChild(new CubeRobot(cube));

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	init = true;
}

Renderer::~Renderer(void) {
	delete root;
	delete quad;
	delete camera;
	delete cube;
	delete shader;
	glDeleteTextures(1, &texture);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	root->Update(dt);
}

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
	BindShader(shader);
	UpdateShaderMatrices();

	for (const auto& i : nodeList) {
		DrawNode(i);
	}

	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
}

void Renderer::DrawDebugNodes() {

	BindShader(debugShader);
	UpdateShaderMatrices();

	for (const auto& i : nodeList) {
		DrawDebugNode(i);
	}

	for (const auto& i : transparentNodeList) {
		DrawDebugNode(i);
	}
}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, model.values);
		glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)& n->GetColour());

		texture = n->GetTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), texture);

		n->Draw(*this);
	}
}

void Renderer::DrawDebugNode(SceneNode* n) {
	if (n->GetBoundingVolume()) {
		if (n->GetBoundingVolume()->GetType() == RECTANGLE) {
			BoundingRectangle* b = static_cast<BoundingRectangle*>(n->GetBoundingVolume());
			Matrix4 model = b->GetWorldPos() * Matrix4::Scale(Vector3(b->GetWidth(), b->GetHeight(), b->GetDepth()));
			glUniformMatrix4fv(glGetUniformLocation(debugShader->GetProgram(), "modelMatrix"), 1, false, model.values);
			debugCube->Draw();
			axis->Draw();
		}
		//Matrix4 model = n->GetBoundingVolume()->GetPosition() * Matrix4::Scale(n->GetBoundingVolume())
	}
}

void Renderer::RenderScene() {
	BuildNodeLists(root);
	SortNodeLists();

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(shader);
	UpdateShaderMatrices();
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);

	BindShader(debugShader);
	UpdateShaderMatrices();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	DrawNodes();
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	DrawDebugNodes();

	ClearNodeLists();
}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}