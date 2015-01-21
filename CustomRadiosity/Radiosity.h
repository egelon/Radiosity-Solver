#ifndef RADIOSITY_H
#define RADIOSITY_H

#include "Mesh.h"
#include "RadiosityFace.h"
#include "Ray.h"
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
	void PrepareUnshotRadiosityValues();
	void calculateRadiosityValues();

	bool doesRayHit(Ray* ray, int j, glm::vec3& hitPoint);
	bool isVisibleFrom(int i, int j);

	bool isVisibleFrom(glm::vec3 point_j, glm::vec3 point_i);

	bool isParallelToFace(Ray* r, int radFaceIndex);

	void setMeshFaceColors();

	int getMaxUnshotRadiosityFaceIndex();


private:

	vector<RadiosityFace> sceneFaces;
	vector<vector<double>> formFactors;

};
#endif