#ifndef RADIOSITY_FACE_H
#define RADIOSITY_FACE_H

#include <glm/vec3.hpp>

struct RadiosityFace
{
	ObjectModel* model;
	int faceIndex;
	glm::vec3 totalRadiosity;
	glm::vec3 emission; // the energy emitted per unit of patch area
	glm::vec3 unshotRadiosity;
};

#endif