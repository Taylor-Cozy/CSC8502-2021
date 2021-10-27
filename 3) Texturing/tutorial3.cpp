#include "../nclGL/window.h"
#include "Renderer.h"

int main() {
	Window w("Texturing!", 1280, 720,false);	 //This is all boring win32 window creation stuff!
	if(!w.HasInitialised()) {
		return -1;
	}
	
	Renderer renderer(w);	//This handles all the boring OGL 3.2 initialisation stuff, and sets up our tutorial!
	if(!renderer.HasInitialised()) {
		return -1;
	}

	float rotate = 0.0f;
	Vector3 position(0, 0, -1500.0f);
	float scale = 100.0f;
	float rotation = 0.0f;

	while(w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
		if(Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT) ) {
			--rotate;
			renderer.UpdateTextureMatrix(rotate);
		}

		if(Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT) ) {
			++rotate;
			renderer.UpdateTextureMatrix(rotate);
		}

		if(Window::GetKeyboard()->KeyTriggered(KEYBOARD_1) ) {
			renderer.ToggleFiltering();
		}

		if(Window::GetKeyboard()->KeyTriggered(KEYBOARD_2) ) {
			renderer.ToggleRepeating();
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_K))
			position.y -= 1.0f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_I))
			position.y += 1.0f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_J))
			position.x -= 1.0f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_L))
			position.x += 1.0f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_O))
			position.z -= 1.0f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_P))
			position.z += 1.0f;

		renderer.SetRotation(rotation);
		renderer.SetScale(scale);
		renderer.SetPosition(position);

		renderer.RenderScene();
		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}
	}

	return 0;
}