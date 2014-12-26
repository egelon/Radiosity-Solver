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

	void SetMVP(glm::mat4 mvp){ModelViewProjectionMatrix = mvp;}

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

	static const string vertexShaderFile;
	static const string fragmentShaderFile;

	ShaderLoader shaderLoader;

	//OpenGL IDs
	GLuint vertexBufferID;
	GLuint shaderProgramID;

public:
	vector<glm::vec4> file_vertices;
	vector<glm::vec3> file_normals;
	vector<GLushort> file_elements;

	glm::mat4 ModelViewProjectionMatrix;
};

#endif