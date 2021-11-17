#include "SceneNode.h"
#include "BoundingSphere.h"
#include "BoundingBox.h"
SceneNode::SceneNode(Mesh* mesh, Vector4 colour) {
	this->mesh = mesh;
	this->colour = colour;
	parent = NULL;
	modelScale = Vector3(1, 1, 1);

	boundingVol = &BoundingBox(Vector3(1,1,1), worldTransform);
	distanceFromCamera = 0.0f;
	texture = 0;

	modelMat.ToIdentity();
	textureMat.ToIdentity();
}

SceneNode::~SceneNode(void) {
	for (unsigned int i = 0; i < children.size(); i++) {
		delete children[i];
	}
}

void SceneNode::AddChild(SceneNode* s) {
	children.push_back(s);
	s->parent = this;
}

void SceneNode::RemoveChild(SceneNode* s) {
	for (auto it = children.begin(); it != children.end(); it++) {
		if (s == *it) {
			delete *it;
			children.erase(it);
		}
	}
}

void SceneNode::Draw(const OGLRenderer& r) {
	if (mesh) { mesh->Draw(); }
}

void SceneNode::Update(float dt) {

	UpdateVariables(dt);

	if (parent) {
		worldTransform = parent->worldTransform * transform;
	}
	else {
		worldTransform = transform;
	}

	if (boundingVol->GetType() == BOX) {
		BoundingBox* b = static_cast<BoundingBox*>(boundingVol);
		//b->SetWorldPosition(Matrix4::Translation(worldTransform.GetPositionVector()) * Matrix4::Translation(b->GetOffset()) * Matrix4::Scale(b->GetBoundingSize()));
		b->SetWorldPosition(worldTransform * Matrix4::Translation(b->GetOffset()) * Matrix4::Scale(b->GetBoundingSize()));
	}
	
	if (boundingVol->GetType() == SPHERE) {
		BoundingSphere* b = static_cast<BoundingSphere*>(boundingVol);
		Vector3 boundingSize(b->GetBoundingRadius(), b->GetBoundingRadius(), b->GetBoundingRadius());

		b->SetWorldPosition(Matrix4::Translation(worldTransform.GetPositionVector()) * Matrix4::Translation(b->GetOffset()) * Matrix4::Scale(boundingSize));
	}

	for (vector<SceneNode*>::iterator i = children.begin(); i != children.end(); i++) {
		(*i)->Update(dt);
	}
}

void SceneNode::SetShaderVariables() {

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, *texture);

	modelMat = GetWorldTransform();
	textureMat.ToZero();
}