#include "CubeRobot.h"
#include "BoundingSphere.h"
#include "BoundingBox.h"

CubeRobot::CubeRobot(Mesh* cube)
{
	SceneNode* body = new SceneNode(cube, Vector4(1, 0, 0, 1));
	body->SetModelScale(Vector3(10, 15, 5));
	body->SetTransform(Matrix4::Translation(Vector3(0, 35, 0)));
	AddChild(body);

	SceneNode* hips = new SceneNode();
	hips->SetModelScale(Vector3(1, 1, 1));
	hips->SetTransform(Matrix4::Translation(Vector3(0, 0, 0)));
	body->AddChild(hips);

	head = new SceneNode(cube, Vector4(0, 1, 0, 1));
	head->SetModelScale(Vector3(5, 5, 5));
	head->SetTransform(Matrix4::Translation(Vector3(0, 30, 0)));
	body->AddChild(head);

	leftArm = new SceneNode(cube, Vector4(0, 0, 1, 1));
	leftArm->SetModelScale(Vector3(3, -18, 3));
	leftArm->SetTransform(Matrix4::Translation(Vector3(-12, 30, -1)));
	body->AddChild(leftArm);

	rightArm = new SceneNode(cube, Vector4(0, 0, 1, 1));
	rightArm->SetModelScale(Vector3(3, -18, 3));
	rightArm->SetTransform(Matrix4::Translation(Vector3(12, 30, -1)));
	body->AddChild(rightArm);

	SceneNode* leftLeg = new SceneNode(cube, Vector4(0, 0, 1, 1));
	leftLeg->SetModelScale(Vector3(3, -17.5, 3));
	leftLeg->SetTransform(Matrix4::Translation(Vector3(-8, 0, 0)));
	hips->AddChild(leftLeg);

	SceneNode* rightLeg = new SceneNode(cube, Vector4(0, 0, 1, 1));
	rightLeg->SetModelScale(Vector3(3, -17.5, 3));
	rightLeg->SetTransform(Matrix4::Translation(Vector3(8, 0, 0)));
	hips->AddChild(rightLeg);

	body->SetBoundingVolume(new BoundingBox(body->GetModelScale(), body->GetWorldTransform()));
	body->GetBoundingVolume()->SetOffset(Vector3(0, body->GetModelScale().y, 0));
	
	head->SetBoundingVolume(new BoundingBox(head->GetModelScale(), head->GetWorldTransform()));
	head->GetBoundingVolume()->SetOffset(Vector3(0, head->GetModelScale().y, 0));
	
	leftArm->SetBoundingVolume(new BoundingBox(leftArm->GetModelScale(), leftArm->GetWorldTransform()));
	leftArm->GetBoundingVolume()->SetOffset(Vector3(0, leftArm->GetModelScale().y, 0));

	rightArm->SetBoundingVolume(new BoundingBox(rightArm->GetModelScale(), rightArm->GetWorldTransform()));
	rightArm->GetBoundingVolume()->SetOffset(Vector3(0, rightArm->GetModelScale().y, 0));

	leftLeg->SetBoundingVolume(new BoundingBox(leftLeg->GetModelScale(), leftLeg->GetWorldTransform()));
	leftLeg->GetBoundingVolume()->SetOffset(Vector3(0, leftLeg->GetModelScale().y, 0));

	rightLeg->SetBoundingVolume(new BoundingBox(rightLeg->GetModelScale(), rightLeg->GetWorldTransform()));
	rightLeg->GetBoundingVolume()->SetOffset(Vector3(0, rightLeg->GetModelScale().y, 0));
}

void CubeRobot::Update(float dt) {
	transform = transform * Matrix4::Rotation(30.0f * dt, Vector3(0, 1, 0));

	head->SetTransform(head->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(0, 1, 0)));
	leftArm->SetTransform(leftArm->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(1, 0, 0)));
	rightArm->SetTransform(rightArm->GetTransform() * Matrix4::Rotation(30.0f * dt, Vector3(1, 0, 0)));

	SceneNode::Update(dt);
}
