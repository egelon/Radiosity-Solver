#include "HalfEdge.h"

HalfEdge::HalfEdge(Vertex* v, Face* f)
{
	vertex = v;
	face = f;
	next = NULL;
	opposite = NULL;
}

HalfEdge::~HalfEdge()
{
	if (opposite != NULL)
		opposite->opposite = NULL;
}

Vertex* HalfEdge::getVertex() const
{ 
	assert (vertex != NULL);
	return vertex;
}

HalfEdge* HalfEdge::getNext() const
{ 
	assert (next != NULL);
	return next;
}

Face* HalfEdge::getFace() const
{ 
	assert (face != NULL);
	return face;
}

HalfEdge* HalfEdge::getOpposite() const
{
	// warning!  the opposite edge might be NULL!
	return opposite;
}

Vertex* HalfEdge::getEndVertex() const
{
	return getNext()->getVertex();
}

Vertex* HalfEdge::operator[](int i) const
{ 
	if (i==0)
		return getVertex();
	if (i==1)
		return getNext()->getVertex();
	assert(0);
}

void HalfEdge::setOpposite(HalfEdge *e)
{
	assert (opposite == NULL); 
	assert (e != NULL);
	assert (e->opposite == NULL);
	opposite = e; 
	e->opposite = this; 
}

void HalfEdge::clearOpposite()
{ 
	if (opposite == NULL)
		return; 
	assert (opposite->opposite == this); 
	opposite->opposite = NULL;
	opposite = NULL; 
}

void HalfEdge::setNext(HalfEdge *e)
{
	assert (next == NULL);
	assert (e != NULL);
	assert (face == e->face);
	next = e;
}