#include "HeightMap.h"
#include <iostream>

HeightMap::HeightMap(const std::string& firstOctave, const std::string& secondOctave, const std::string& thirdOctave, const std::string& squareGradient)
{
	type = GL_TRIANGLES;

	int iWidth, iHeight, iChans;
	unsigned char* firstOct = SOIL_load_image(firstOctave.c_str(), &iWidth, &iHeight, &iChans, 1);
	unsigned char* secondOct = SOIL_load_image(secondOctave.c_str(), &iWidth, &iHeight, &iChans, 1);
	unsigned char* thirdOct = SOIL_load_image(thirdOctave.c_str(), &iWidth, &iHeight, &iChans, 1);
	unsigned char* squareGrad = SOIL_load_image(squareGradient.c_str(), &iWidth, &iHeight, &iChans, 1);

	if (!firstOct || !secondOct || !thirdOct) {
		std::cout << "Heightmap couldn't be loaded!" << std::endl;
		return;
	}

	width = iWidth;
	numVertices = iWidth * iHeight;
	numIndices = (iWidth - 1) * (iHeight - 1) * 6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];
	colours = new Vector4[numVertices];

	Vector3 vertexScale = Vector3(16.0f, 2.5f, 16.0f);
	Vector2 textureScale = Vector2(1 / 16.0f, 1 / 16.0f);

	for (int z = 0; z < iHeight; z++) {
		for (int x = 0; x < iWidth; x++) {
			int offset = (z * iWidth) + x;
			//std::cout << (int)firstOct[offset] << std::endl;
			float height = ((0.75 * firstOct[offset]) + (0.25 * secondOct[offset]) + (0.0 * thirdOct[offset])) - squareGrad[offset];
			heights.push_back(height * vertexScale.y);
			vertices[offset] = Vector3(x, height, z) * vertexScale;
			textureCoords[offset] = Vector2(x, z) * textureScale;
			colours[offset] = Vector4(height / 255.0f, 0, 0, 1);
		}
	}
	SOIL_free_image_data(firstOct);
	SOIL_free_image_data(secondOct);
	SOIL_free_image_data(thirdOct);
	SOIL_free_image_data(squareGrad);

	int i = 0;

	for (int z = 0; z < iHeight - 1; z++) {
		for (int x = 0; x < iWidth - 1; x++) {
			int a = (z * iWidth) + x;
			int b = (z * iWidth) + (x + 1);
			int c = ((z + 1) * iWidth) + (x + 1);
			int d = ((z + 1) * iWidth) + x;

			indices[i++] = a;
			indices[i++] = c;
			indices[i++] = b;

			indices[i++] = c;
			indices[i++] = a;
			indices[i++] = d;
		}
	}

	GenerateNormals();
	GenerateTangents();
	BufferData();

	heightmapSize.x = vertexScale.x * (iWidth - 1);
	heightmapSize.y = vertexScale.y * 255.0f;
	heightmapSize.z = vertexScale.z * (iHeight - 1);
}

