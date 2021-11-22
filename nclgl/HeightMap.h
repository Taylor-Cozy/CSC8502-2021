#pragma once
#include "Mesh.h"
#include <string>

class HeightMap : public Mesh
{
public:
	HeightMap(const std::string& firstOctave, const std::string& secondOctave, const std::string& thirdOctave, const std::string& squareGradient);
	~HeightMap(void) {};

	Vector3 GetHeightMapSize() const { return heightmapSize; }
	float GetHeightAtLocation(Vector3 location) const {
		return heights[((int)(location.z/16.0f) * width) + (int)(location.x/16.0f)];
	};
protected:

	Vector3 heightmapSize;
	vector<float> heights;
	int width;
};

