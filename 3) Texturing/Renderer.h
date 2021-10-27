#pragma once

#include "../nclgl/OGLRenderer.h"
#include "..//nclgl/Camera.h"

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	virtual ~Renderer(void);

	virtual void RenderScene();

	void UpdateTextureMatrix(float rotation);
	void ToggleRepeating();
	void ToggleFiltering();

	inline void SetScale(float s) { scale = s; }
	inline void SetRotation(float r) { rotation = r; }
	inline void SetPosition(Vector3 p) { position = p; }

	void SwitchToPerspective();
	virtual void UpdateScene(float dt);

protected:
	Shader* shader;
	Mesh* triangle;
	GLuint texture;
	GLuint secondaryTexture;
	bool filtering;
	bool repeating;

	Camera* cam;
	float scale;
	float rotation;
	Vector3 position;
};

