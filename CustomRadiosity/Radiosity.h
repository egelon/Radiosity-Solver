#ifndef RADIOSITY_H
#define RADIOSITY_H

#include "Mesh.h"
#include "RadiosityFace.h"
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include<glm/gtx/vector_angle.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/normal.hpp>

using namespace std;
class Radiosity
{
public:
	void loadSceneFacesFromMesh(Mesh* mesh);
	void initEmittedEnergies();
	void initRadiosityValues();
	void calculateFormFactorsForFace(int i, int samplePoints);
	void calculateAllFormFactors();
	void calculateRadiosityValues();

	void setMeshFaceColors();

	int getMaxUnshotRadiosityFaceIndex();


private:
	vector<RadiosityFace> sceneFaces;
	vector<vector<float>> formFactors;

};
#endif