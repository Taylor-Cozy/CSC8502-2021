#include "CubeRobot.h"
#include "BoundingVolume.h"
#include "BoundingRectangle.h"

CubeRobot::CubeRobot(Mesh* cube)
{
	SceneNode* body = new SceneNode(cube, Vector4(1, 0, 0, 1));
	body->SetModelScale(Vector3(10, 15, 5));
	body->SetTransform(Matrix4::Translation(Vector3(0, 35, 0)));
	AddChild(body);
	body->SetBoundingVolume(new BoundingRectangle(body->GetModelScale() * 2, body->GetTransform().GetPositionVector()));

	SceneNode* hips = new SceneNode();
	hips->SetModelScale(Vector3(1, 1, 1));
	hips->SetTransform(Matrix4::Translation(Vector3(0, 0, 0)));
	body->AddChild(hips);
	hips->SetMesh(Mesh::GenerateTriangle());
	hips->SetColour(Vector4(0, 1, 0, 1));

	head = new SceneNode(cube, Vector4(0, 1, 0, 1));
	head->SetModelScale(Vector3(5, 5, 5));
	head->SetTransform(Matrix4::Translation(Vector3(0, 30, 0)));
	body->AddChild(head);
	head->SetBoundingVolume(new BoundingRectangle(head->GetModelScale() * 2, head->GetTransform().GetPositionVector()));

	leftArm = new SceneNode(cube, Vector4(0, 0, 1, 1));
	leftArm->SetModelScale(Vector3(3, -18, 3));
	leftArm->SetTransform(Matrix4::Translation(Vector3(-12, 30, -1)));
	body->AddChild(leftArm);
	leftArm->SetBoundingVolume(new BoundingRectangle(leftArm->GetModelScale() * 2, leftArm->GetTransform().GetPositionVector()));

	rightArm = new SceneNode(cube, Vector4(0, 0, 1, 1));
	rightArm->SetModelScale(Vector3(3, -18, 3));
	rightArm->SetTransform(Matrix4::Translation(Vector3(12, 30, -1)));
	body->AddChild(rightArm);
	rightArm->SetBoundingVolume(new BoundingRectangle(rightArm->GetModelScale() * 2, rightArm->GetTransform().GetPositionVector()));

	SceneNode* leftLeg = new SceneNode(cube, Vector4(0, 0, 1, 1));
	leftLeg->SetModelScale(Vector3(3, -17.5, 3));
	leftLeg->SetTransform(Matrix4::Translation(Vector3(-8, 0, 0)));
	hips->AddChild(leftLeg);
	leftLeg->SetBoundingVolume(new BoundingRectangle(leftLeg->GetModelScale() * 2, leftLeg->GetTransform().GetPositionVector()));

	SceneNode* rightLeg = new SceneNode(cube, Vector4(0, 0, 1, 1));
	rightLeg->SetModelScale(Vector3(3, -17.5, 3));
	rightLeg->SetTransform(Matrix4::Translation(Vector3(8, 0, 0)));
	hips->AddChild(rightLeg);
	rightLeg->SetBoundingVolume(new BoundingRectangle(rightLeg->GetModelScale() * 2, rightLeg->GetTransform().GetPositionVector()));
}

void CubeRobot::Update(float dt) {
	transform = transform * Matrix4::Rotation(30.0f * dt, Vector3(0, 1, 0));

	head->SetTransform(head->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(0, 1, 0)));
	leftArm->SetTransform(leftArm->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(1, 0, 0)));
	rightArm->SetTransform(rightArm->GetTransform() * Matrix4::Rotation(30.0f * dt, Vector3(1, 0, 0)));

	SceneNode::Update(dt);
}
