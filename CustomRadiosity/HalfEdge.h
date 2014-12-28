#ifndef HALFEDGE_H
#define HALFEDGE_H

#include "Face.h"
#include "Vertex.h"
#include <glm/vec3.hpp>

class Face;
class Vertex;

struct HalfEdge
{
	public:
	HalfEdge(Vertex* v, Face* f);
	HalfEdge(const HalfEdge&);
	HalfEdge* Clone() const;
	~HalfEdge();

	Vertex* getVertex() const;
	HalfEdge* getNext() const;
	Face* getFace() const;
	HalfEdge* getOpposite() const;

	Vertex* getEndVertex() const;

	Vertex* operator[](int i) const;

	void setOpposite(HalfEdge *e);
	void clearOpposite();
	void setNext(HalfEdge *e);


	private:
	HalfEdge& operator=(const HalfEdge&) {assert(0);}

	Vertex *vertex;
	Face *face;
	HalfEdge *opposite;
	HalfEdge *next;
};

#endif