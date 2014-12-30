#ifndef SCENEMODEL_H
#define SCENEMODEL_H

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>

#include <vector>
#include <string>

#include "ModelFace.h"

using namespace std;
struct ObjectModel
{
	vector<glm::vec3> vertices;
	vector<glm::vec3> textureUVW;
	vector<glm::vec3> vertexNormals;
	vector<ModelFace> faces;
};

#endif