#include "../NCLGL/window.h"
#include "Renderer.h"

int main()	{
	Window w("Make your own project!", 1600, 900, false);

	if(!w.HasInitialised()) {
		return -1;
	}
	
	Renderer renderer(w);
	if(!renderer.HasInitialised()) {
		return -1;
	}

	w.LockMouseToWindow(true);
	w.ShowOSPointer(false);

	while(w.UpdateWindow()  && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_2)) {
			renderer.ToggleTrack(false);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_1)) {
			renderer.ToggleTrack(true);
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F)) {
			renderer.ActivatePortal();
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_X)) {
			renderer.ToggleDebug();
		}


		w.SetTitle("Make your own project! " + std::to_string(1.0f/ w.GetTimer()->GetTimeDeltaSeconds()));
	}
	return 0;
}