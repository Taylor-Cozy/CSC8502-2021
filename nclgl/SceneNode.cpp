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
	diffuseTex = 0;

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
	glBindTexture(GL_TEXTURE_2D, *diffuseTex);

	//glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseBump"), 1);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, *diffuseBump);

	if (useSecond) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "secondaryTex"), 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, *secondaryTex);

		//glUniform1i(glGetUniformLocation(shader->GetProgram(), "secondaryBump"), 3);
		//glActiveTexture(GL_TEXTURE3);
		//glBindTexture(GL_TEXTURE_2D, *secondaryBump);
	}

	if (useThird) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "thirdTex"), 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, *thirdTex);

		glUniform1i(glGetUniformLocation(shader->GetProgram(), "thirdBump"), 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, *thirdBump);
	}

	modelMat = GetWorldTransform();
	textureMat.ToZero();
}