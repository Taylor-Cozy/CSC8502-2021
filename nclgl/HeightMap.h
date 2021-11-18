#pragma once
#include "Mesh.h"
#include <string>

class HeightMap : public Mesh
{
public:
	HeightMap(const std::string& firstOctave, const std::string& secondOctave, const std::string& thirdOctave, const std::string& squareGradient);
	~HeightMap(void) {};

	Vector3 GetHeightMapSize() const { return heightmapSize; }
protected:
	float GenerateHeight(int x, int z, int iWidth, int iHeight);

	Vector3 heightmapSize;
};

