#ifndef FACE_H
#define FACE_H

#include "HalfEdge.h"
#include "Vertex.h"

class HalfEdge;
class Vertex;

#include <glm/vec3.hpp>

class Face
{
	public:
	Face(const glm::vec3& c, const glm::vec3& e);
	Face(const Face&);
	Face* Clone() const;
	~Face();

	Vertex* operator[](int i) const;
	glm::vec3 getCentroid() const;

	HalfEdge* getEdge() const;
	void setEdge(HalfEdge *e);

	glm::vec3 getColor() const;
	glm::vec3 getEmission() const;

	glm::vec3 getNormal() const;
	void setEmission(glm::vec3 e);

	float getArea() const;



	private:
	Face& operator = (const Face &f) { assert(0); }

	HalfEdge* edge;

	glm::vec3 emission;
	glm::vec3 color;
};

#endif