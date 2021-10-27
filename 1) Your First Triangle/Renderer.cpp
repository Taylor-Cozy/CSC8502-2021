#include "Renderer.h"
#include <chrono>
#include <iostream>

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	//triangle = Mesh::GenerateTriangle();
	triangle = Mesh::GenerateQuad();

	basicShader = new Shader("BasicVertex.glsl", "ColourFragment.glsl");

	if (!basicShader->LoadSuccess()) {
		return;
	}
	init = true;
}

Renderer::~Renderer(void) {
	delete triangle;
	delete basicShader;
}

void Renderer::RenderScene() {
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	BindShader(basicShader);
	triangle->Draw();

	glBindBuffer(GL_ARRAY_BUFFER, triangle->getBufferObject()[COLOUR_BUFFER]);
	Vector4* colourBuf = (Vector4*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

	auto x = std::chrono::system_clock::now();
	std::cout << sin(x.time_since_epoch().count() / 10000000) << std::endl;

	for (int i = 0; i < 4; i++) {
		colourBuf->x = abs(sin(x.time_since_epoch().count() / 10000000));
		colourBuf->y = abs(cos(x.time_since_epoch().count() / 10000000));;
		colourBuf->z = abs(tan(x.time_since_epoch().count() / 10000000));;
		colourBuf++;
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);


	glBindBuffer(GL_ARRAY_BUFFER, triangle->getBufferObject()[VERTEX_BUFFER]);
	Vector3* vertBuf = (Vector3*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

	for (int i = 0; i < 4; i++) {
		vertBuf->x *= 0.99f;
		vertBuf->y *= 0.99f;
		vertBuf++;
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);


}