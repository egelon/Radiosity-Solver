#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include <GL/glew.h>

#include <string>
using namespace std;

class ShaderLoader
{
public:
	GLuint LoadShaders(string vertex_file, string fragment_file);
};

#endif