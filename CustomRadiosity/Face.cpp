#include "Face.h"

#include <vector>
#include <glm/glm.hpp>
using namespace std;

float AreaOfTriangle(float a, float b, float c)
{
	// Area of Triangle =  (using Heron's Formula)
	//  sqrt[s*(s-a)*(s-b)*(s-c)]
	//    where s = (a+b+c)/2
	float s = (a+b+c) / (float)2;
	return sqrt(s*(s-a)*(s-b)*(s-c));
}

float AreaOfTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
	float sideA = glm::distance (a ,b);
	float sideB = glm::distance (b ,c);
	float sideC = glm::distance (c ,a);
	
	float area = (sideA + sideB + sideC) * 0.5;
	
	return sqrt(area * (area - sideA) * (area - sideB) * (area - sideC));
}


Face::Face(const glm::vec3 &c, const glm::vec3 &e)
{
	edge = NULL;
	color = c; 
	emission = e; 
}

Face::~Face() {}

Vertex* Face::operator[](int i) const 
{ 
	assert (edge != NULL);

	if (i==0) 
		return edge->getVertex();

	if (i==1) 
		return edge->getNext()->getVertex();

	if (i==2) 
		return edge->getNext()->getNext()->getVertex();

	assert(0);
}

glm::vec3 Face::getCentroid() const
{
	glm::vec3 a = (*this)[0]->getPosition();
	glm::vec3 b = (*this)[1]->getPosition();
	glm::vec3 c = (*this)[2]->getPosition();

	glm::vec3 result (
			(a.x + b.x + c.x) / 3.0f,
			(a.y + b.y + c.y) / 3.0f,
			(a.z + b.z + c.z) / 3.0f
		);

	return result;
}

glm::vec3 Face::getNormal() const
{
	glm::vec3 a = (*this)[0]->getPosition();
	glm::vec3 b = (*this)[1]->getPosition();
	glm::vec3 c = (*this)[2]->getPosition();
	
	return glm::normalize(glm::cross(c - a, b - a));
}

HalfEdge* Face::getEdge() const
{ 
	assert (edge != NULL);
	return edge; 
}

void Face::setEdge(HalfEdge *e)
{
	assert (edge == NULL);
	edge = e;
}

glm::vec3 Face::getColor() const { return color; }
glm::vec3 Face::getEmission() const { return emission; }
void Face::setEmission(glm::vec3 e) {emission = e;}

float Face::getArea() const 
{
	HalfEdge* startEdge = getEdge();
	Vertex* start = startEdge->getVertex();

	vector<Vertex*> vertices;
	vertices.push_back(start);

	HalfEdge* currentEdge = startEdge->getNext();
	while(currentEdge->getVertex() != start)
	{
		vertices.push_back(currentEdge->getVertex());
		currentEdge = currentEdge->getNext();
	}

	int numPoints = vertices.size();
	int j = numPoints-1;  // The last vertex is the 'previous' one to the first
	
	float area = 0;       // Accumulates area in the loop
	
	for (int i=0; i<numPoints; i++)
	{ 
		area += (vertices[j]->x() + vertices[i]->x()) * (vertices[j]->y() - vertices[i]->y()); 
		j = i;
	}
	return area/2;
}

