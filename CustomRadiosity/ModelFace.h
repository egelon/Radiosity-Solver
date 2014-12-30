#ifndef MODELFACE_H
#define MODELFACE_H

struct ModelFace
{
	vector<GLuint> vertexIndexes;
	vector<GLuint> textureIndexes;
	vector<GLuint> normalIndexes;
};

#endif