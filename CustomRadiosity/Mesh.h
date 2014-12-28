#ifndef MESH_H
#define MESH_H

#include "Vertex.h"
#include "HalfEdge.h"
#include "Face.h"
#include "BoundingBox.h"
#include "ShaderLoader.h"

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
	void PrepareToDraw();
	void DrawWireframe();
	void Cleanup();
	void Draw();

	int numVertices() const;
	int numHalfEdges() const;
	int numFaces() const;

	Vertex* getVertexByIndex(int index) const;
	HalfEdge* getEdge(Vertex *a, Vertex *b) const; //edge that starts from a and goes to b

	vector<Vertex*> getVertices() const;
	vector<HalfEdge*> getHalfEdges() const;
	vector<Face*> getFaces() const;

	GLuint LoadShaders();

	void SetMVP(glm::mat4 mvp){ ModelViewProjectionMatrix = mvp; }


	vector<GLfloat> GLF_getVertexPositions() { return vertex_positions; };
	vector<GLfloat> GLF_getVertexColors() { return vertex_colors; };
	vector<GLuint> GLUI_getFaceIndexes() { return face_indexes; };
	vector<GLfloat> GLF_getFaceNormals() { return face_normals; };

	void cacheVertexPositions();
	void cacheVertexColors();
	void cacheFaceIndexes();
	void cacheFaceNormals();


private:
	void LoadToArrays(string input_file);
	void LoadFromArrays();

	void addFace(Vertex *a, Vertex *b, Vertex *c, const glm::vec3& col, const glm::vec3& emit);
	void removeFace(Face *f);
	
	Vertex* addVertex(const glm::vec3& position);

	vector<Vertex*> vertices;
	vector<HalfEdge*> halfEdges;
	vector<Face*> faces;
	BoundingBox* bbox;

	vector<GLfloat> vertex_positions;
	vector<GLfloat> vertex_colors;
	vector<GLuint> face_indexes;
	vector<GLfloat> face_normals;

	vector<glm::vec3> file_vertices;
	vector<glm::vec3> file_colors;
	vector<glm::vec3> file_normals;
	vector<GLuint> file_elements;

	glm::mat4 ModelViewProjectionMatrix;

	static const string vertexShaderFile;
	static const string fragmentShaderFile;

	ShaderLoader shaderLoader;

	//OpenGL IDs
	GLuint vertexBufferID;
	GLuint colorBufferID;
	GLuint elementBufferID;
	GLuint shaderProgramID;
};

#endif