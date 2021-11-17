#include "WaterNode.h"

WaterNode::WaterNode(Mesh* m, Vector4 colour) : SceneNode(m, colour)
{
}

WaterNode::~WaterNode(void) {}

void WaterNode::Update(float dt) {
	waterCycle += dt * 0.025f;
	waterRotate += dt * 0.20f;
	time += dt;
}

void WaterNode::Draw(const OGLRenderer& r) {

	glUniform3fv(glGetUniformLocation(shader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "heightmapTex"), 3);

	glUniform1f(glGetUniformLocation(reflectShader->GetProgram(), "time"), time);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, *texture);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, *cubeTex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, *bumpTex);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, *heightMapTex);
}