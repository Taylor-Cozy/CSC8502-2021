#include "Camera.h"
#include "Window.h"
#include <algorithm>

void Camera::UpdateCamera(float dt)
{

	pitch -= (Window::GetMouse()->GetRelativePosition().y);
	yaw -= (Window::GetMouse()->GetRelativePosition().x);

	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);

	if (yaw < 0) {
		yaw += 360.0f;
	}
	if (yaw > 360) {
		yaw -= 360.0f;
	}


	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));

	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);

	

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
		position += forward * speed * dt;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
		position -= forward * speed * dt;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
		position -= right * speed * dt;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
		position += right * speed * dt;
	}


	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
		if (!disableMove) {
			position.y -= speed * dt;
		}
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
		if (!disableMove) {
			position.y += speed * dt;
		}
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_Q)) {
		roll -= speed * dt;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_E)) {
		roll += speed * dt;
	}

	if (roll < 0) {
		roll += 360.0f;
	}
	if (roll > 360) {
		roll -= 360.0f;
	}

	if (disableMove) {
		roll = 0;
		pitch = -90.0f;
	}
}

Matrix4 Camera::BuildViewMatrix()
{
	return Matrix4::Rotation(-roll, Vector3(0, 0, -1)) * Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
		Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
		Matrix4::Translation(-position);
}
