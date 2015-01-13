#ifndef RADIOSITY_FACE_H
#define RADIOSITY_FACE_H

#include <glm/vec3.hpp>

struct RadiosityFace
{
	ObjectModel* model;
	int faceIndex;
	glm::dvec3 totalRadiosity;
	glm::dvec3 emission; // the energy emitted per unit of patch area
	glm::dvec3 unshotRadiosity;
};

#endif