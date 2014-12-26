#ifndef MESH_H
#define MESH_H

#include "Vertex.h"
#include "HalfEdge.h"
#include "Face.h"
#include "BoundingBox.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <string>

// Include GLEW
#include <GL/glew.h>
#include <glfw/glfw3.h>

using namespace std;

class Mesh 
{
	public:
	Mesh();
	virtual ~Mesh();

	void LoadToArrays(string input_file, vector<glm::vec4>& file_vertices, vector<glm::vec3>& file_normals, vector<GLushort>& file_elements);
	void LoadFromArrays(vector<glm::vec4>& file_vertices, vector<glm::vec3>& file_normals, vector<GLushort>& file_elements);

	void Subdivide();
	void DrawWireframe();
	void Draw();

	int numVertices() const;
	int numHalfEdges() const;
	int numFaces() const;

	Vertex* getVertexByIndex(int index) const;
	HalfEdge* getEdge(Vertex *a, Vertex *b) const; //edge that starts from a and goes to b

	vector<Vertex*> getVertices() const;
	vector<HalfEdge*> getHalfEdges() const;
	vector<Face*> getFaces() const;

	private:
	void addFace(Vertex *a, Vertex *b, Vertex *c, const glm::vec3& col, const glm::vec3& emit);
	void removeFace(Face *f);
	
	Vertex* addVertex(const glm::vec3& position);

	vector<Vertex*> vertices;
	vector<HalfEdge*> halfEdges;
	vector<Face*> faces;
	BoundingBox* bbox;
};

#endif