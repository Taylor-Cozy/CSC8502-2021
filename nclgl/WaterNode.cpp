#include "WaterNode.h"

WaterNode::WaterNode(Vector3 hSize, Mesh* m, Vector4 colour) : SceneNode(m, colour), hSize(hSize)
{
}

WaterNode::~WaterNode(void) {}

void WaterNode::UpdateVariables(float dt) {

	waterCycle += dt * 0.025f;
	waterRotate += dt * 0.20f;
	time += dt;
}

void WaterNode::SetShaderVariables()
{
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "cubeTex"), 2);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "heightmapTex"), 3);

	glUniform1f(glGetUniformLocation(shader->GetProgram(), "time"), time);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, *diffuseTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, *cubeTex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, *bumpTex);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, *heightMapTex);

	modelMat =
		Matrix4::Translation(Vector3(hSize.x * 0.5f, hSize.y * 0.2f, hSize.z * 0.5f)) * Matrix4::Translation(Vector3(0, sin(time), 0)) *
		Matrix4::Scale(hSize * 0.5f) *
		Matrix4::Rotation(90, Vector3(1, 0, 0));

	textureMat =
		Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
		Matrix4::Scale(Vector3(20, 20, 20)) *
		Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));
}
