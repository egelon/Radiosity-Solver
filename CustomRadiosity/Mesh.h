#ifndef MESH_H
#define MESH_H

#include "ShaderLoader.h"
#include "SceneObject.h"
#include "Material.h"

#include <GL/glew.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>

#include <string>
#include <vector>
using namespace std;


class Mesh 
{
public:
	Mesh();
	virtual ~Mesh();

	
	void Load(string input_file);

	void Subdivide();
	void ResetMesh();
	void PrepareToDraw();
	void DrawWireframe();
	void Cleanup();
	void Draw();

	vector<ModelFace*> GetFaceIndexesFromVertexIndex(int modelIndex, int vertIndex);

	void OutputToBitmap(string bmpName, int width, int height);


	GLuint LoadDefaultShaders();
	GLuint LoadGouraudShaders();

	void SetMVP(glm::mat4 mvp){ ModelViewProjectionMatrix = mvp; }

	vector<GLfloat> GLF_getVertexPositions() { return vertex_positions; };
	vector<GLfloat> GLF_getVertexColors() { return vertex_colors; };
	vector<GLuint> GLUI_getFaceIndexes() { return face_indexes; };
	vector<GLfloat> GLF_getFaceNormals() { return face_normals; };

	void cacheVerticesFacesAndColors();
	void cacheVerticesFacesAndColors_Radiosity();
	void cacheVerticesFacesAndColors_Radiosity_II();


private:

	void parseObject(ifstream& fileStream, SceneObject& currentObject, int totalVertexCount);

	void parseMaterials(string materialsFileName);
	Material* getMaterialPtrByName(string matName);

	vector<GLfloat> vertex_positions;
	vector<GLfloat> vertex_colors;
	vector<GLuint> face_indexes;
	vector<GLfloat> face_normals;

public:	
	vector<SceneObject> sceneModel;
private:	
	vector<SceneObject> startingSceneModel;

	vector<Material> materials;

	glm::mat4 ModelViewProjectionMatrix;

	ShaderLoader shaderLoader;

	//OpenGL IDs
	GLuint vertexBufferID;
	GLuint colorBufferID;
	GLuint elementBufferID;
	GLuint shaderProgramID;
};

#endif