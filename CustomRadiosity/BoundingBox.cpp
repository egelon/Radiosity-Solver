#include "BoundingBox.h"
#include "functions.h"
#include <windows.h>
#include <GL/glut.h>
#include <GL/glu.h>

BoundingBox::BoundingBox(glm::vec3 _min, glm::vec3 _max)
{
	Set(_min,_max);
}
BoundingBox::~BoundingBox() {}

glm::vec3 BoundingBox::getMin() const { return min; }
glm::vec3 BoundingBox::getMax() const { return max; }

glm::vec3 BoundingBox::getCenter() const
{
	glm::vec3 center( (min.x + max.x) / 2.0, (min.y + max.y) / 2.0, (min.z + max.z) / 2.0);
	return center;
}

float BoundingBox::maxDim() const 
{
	float x = max.x - min.x;
	float y = max.y - min.y;
	float z = max.z - min.z;
	return max3(x,y,z);
}

void BoundingBox::Set(BoundingBox *bb)
{
	assert (bb != NULL);
	min = bb->min;
	max = bb->max;
}

void BoundingBox::Set(glm::vec3 _min, glm::vec3 _max)
{
	assert (min.x <= max.x &&
		min.y <= max.y &&
		min.z <= max.z);
	min = _min;
	max = _max;
}

void BoundingBox::Extend(const glm::vec3 v)
{
	min = glm::vec3(min2(min.x, v.x), min2(min.y ,v.y), min2(min.z ,v.z));
	max = glm::vec3(max2(max.x, v.x), max2(max.y, v.y), max2(max.z, v.z));
}

void BoundingBox::Extend(BoundingBox *bb) 
{
	assert (bb != NULL);
	Extend(bb->getMin());
	Extend(bb->getMax());
}

void BoundingBox::Draw() const
{
	// draw a wireframe box to represent the boundingbox

	glColor3f(1,1,1);
	glLineWidth(1);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);

	glVertex3f(min.x, min.y, min.z);
	glVertex3f(max.x, min.y, min.z);
	glVertex3f(min.x, min.y, min.z);
	glVertex3f(min.x, max.y, min.z);
	glVertex3f(max.x, max.y, min.z);
	glVertex3f(max.x, min.y, min.z);
	glVertex3f(max.x, max.y, min.z);
	glVertex3f(min.x, max.y, min.z);

	glVertex3f(min.x, min.y, min.z);
	glVertex3f(min.x, min.y, max.z);
	glVertex3f(min.x, max.y, min.z);
	glVertex3f(min.x, max.y, max.z);
	glVertex3f(max.x, min.y, min.z);
	glVertex3f(max.x, min.y, max.z);
	glVertex3f(max.x, max.y, min.z);
	glVertex3f(max.x, max.y, max.z);

	glVertex3f(min.x, min.y, max.z);
	glVertex3f(max.x, min.y, max.z);
	glVertex3f(min.x, min.y, max.z);
	glVertex3f(min.x, max.y, max.z);
	glVertex3f(max.x, max.y, max.z);
	glVertex3f(max.x, min.y, max.z);
	glVertex3f(max.x, max.y, max.z);
	glVertex3f(min.x, max.y, max.z);

	glEnd();
	glEnable(GL_LIGHTING);
}